#include "../inc/post.class.hpp"

std::string intToString(int num) {
  std::stringstream ss;
  ss << num;
  return ss.str();
}

void    memory_freeder(char **args, int args_size)
{
    for(int i = 0; i < args_size; i++)
    {
        if (args[i])    
            free(args[i]);
    }
    free(args);
}

char** Post::fill_cgi_env(t_client* client)
{
    client->response->cgi_pipe[0] = client->request->file;
    char** args = (char**) malloc(4 * sizeof(char*));
    args[0] = strdup("REQUEST_METHOD=POST");
    std::string arg = "CONTENT_TYPE=" + client->request->request_map.at("content-type");
    args[1] = strdup(arg.c_str());
    arg.clear();
    arg = "CONTENT_LENTGH=" + intToString(client->request->body_size);
    args[2] = strdup(arg.c_str());
    arg.clear();
    args[3] = strdup(arg.c_str());
    arg.clear();
    args[4] = NULL;
    return args;
}

void Post::client_served(t_client* client)
{
    t_request* req = client->request;
    std::cout << "Client's request is SERVED\n";
    client->state = WAITING;
    if (client->response->is_cgi)
    {
        client->response->cgi_pipe[0] = req->file;
        serve_cgi(client, fill_cgi_env(client), 4);
    }
    else
        fill_response(client, 201, "Created!", true);
    close(req->file);
    req->first_time = false;
    req->body_size = 0;
}

Post::Post(void) {}

void    Post::create_file(t_client* client)
{
    if (!client->response->is_cgi)
    {
        std::string filename;
        if (!client->request->filename.empty())
            filename.append(client->request->filename);
        else
            filename.append("file");
        for (int i = 0; i < INT_MAX; i++)
        {
            
            std::string file_path = getwd(NULL) + client->response->rootfilepath;
            file_path.append(filename);
            if (i)
                file_path.append(intToString(i));
            file_path.append(client->request->extension);
            if (access(file_path.c_str(), F_OK))
            {
                client->request->file = open(file_path.c_str(), O_CREAT | O_APPEND | O_RDWR);
                break;
            }
            file_path.clear();
        }
    }
    else
    {
        std::string filepath = client->response->rootfilepath + "tmp";
        client->request->file = open(filepath.c_str(), O_CREAT | O_TRUNC | O_RDWR);
    }
}

void    Post::serve_cgi(t_client* client, char** args, int args_size)
{
    int status;
    if (client->response->cgi_pipe[0] == UNDEFINED && !pipe(client->response->cgi_pipe)) // pipe not piped 
            std::cout << GREEN_BOLD << "PIPED!" << std::endl;
    if (!fork())
    {
        if (client->response->cgi_pipe[0])
            dup2(client->response->cgi_pipe[0], 0);
        if (client->response->cgi_pipe[1] != 1)
            dup2(client->response->cgi_pipe[1], 1);
        if (execve(args[0], args, NULL))
            std::cout << "FAILED! - " << strerror(errno) << std::endl;
        std::cout << "EXECVE NOT WORKING" << std::endl;
    }
    memory_freeder(args, args_size);
    if (client->response->cgi_pipe[0])
        close(client->response->cgi_pipe[0]);
    if (client->response->cgi_pipe[1] != 1)
        close(client->response->cgi_pipe[1]);
    waitpid(-1, &status, WNOHANG);
}

void    Post::fill_response(t_client *client, int code, std::string status_line, bool write_it)
{
    t_request *req;
    t_response *res;
    std::map<std::string, std::string>  *request_map;
    std::string connection;
    CLIENT_STATE    new_state;

    req = client->request;
    res = client->response;
    request_map = &req->request_map;
    res->http_version = HTTP_VERSION;
    res->status_code = std::to_string(code);
    res->status_line = status_line ;
    connection = req->get_param("connection"); // used for keep-alive
    if (sz(connection)) // type of connection
        res->add("connection", "closed");
    if (write_it)
        res->write_response_in_socketfd(client->fd);
}

void Post::serve_client(t_client *client)
{

    char buff[MAX_BUFFER_SIZE];
    int rbytes = read(client->fd, buff, MAX_BUFFER_SIZE);
    if (!rbytes)
    {
        client->state = WAITING;
        return;
    }
    if (rbytes > 0)
        client->request->body.append(buff, rbytes);
    if (client->request->first_time)
    {
        create_file(client);
        if (client->request->request_map.find("transfer-encoding") == client->request->request_map.end())
            client->request->encod_stat = true;
        else
            client->request->encod_stat = false;
        if (client->request->file < 0)
        {
            fill_response(client, 501, "Internal Server Error", true);
            client->state = WAITING;
            return;
        }
        client->request->first_time = 0;
    }
    if (client->request->encod_stat)
        parse_non_chunked_body(client);
    else
        parse_chunked_body(client);
}

void    Post::parse_non_chunked_body(t_client* client)
{
    t_request* req = client->request;
    int cl = std::atoi(req->request_map.at("content-length").c_str());
    int lsize = req->body.size();
    if (req->body_size + lsize < cl)
    {
        write(req->file, req->body.c_str(), lsize);
        req->body_size += lsize;
        req->body.clear();
    }
    else
    {
        write(req->file, req->body.c_str(), cl - req->body_size);
        req->body.clear();
        client_served(client);
    }
}

void    Post::parse_chunked_body(t_client* client)
{
    t_request* req = client->request;
    size_t pos = 0;
    while(true)
    {
        if (req->endwr) {req->body.erase(0, 1); req->endwr = false;}
        if (!req->data)
        {
            pos = req->body.find("\r\n");
            if (pos == std::string::npos)
                break;
            req->convert_hex(req->body.substr(0, pos));
            if (client->request->hex + client->request->body_size > client->response->configs->max_body_size)
            {
                fill_response(client, 413, "Payload Too Large", true);
                req->body.clear();
                client->state = WAITING;
                break;
            }
            if (!req->hex) 
            {
                req->body.clear();
                client_served(client);
                break;
            }
            req->body.erase(0, pos + 2);
            req->data = true;
        }
        if (req->data)
        {
            int lsize = req->body.size();
            if (req->hex >= lsize)
            {
                write(req->file, req->body.c_str(), lsize);
                req->body.clear();
                req->hex -= lsize;
                req->body_size += lsize;
                break;
            }
            else
            {
                write(req->file, req->body.c_str(), req->hex);
                req->body_size += req->hex;
                req->data = false;
                if (req->hex + 2 <= lsize)
                    req->body.erase(0, req->hex + 2);
                else
                {
                    req->body.clear();
                    req->endwr = true;
                    break;
                }
            }
        }
    }
}

Post::~Post(void) {}