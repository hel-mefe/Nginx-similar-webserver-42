#include "../inc/post.class.hpp"
#include <sys/time.h>
#include <signal.h>

Post::Post(void) {}

std::string intToString(int num) {
  std::stringstream ss;
  ss << num;
  return ss.str();
}

void    memory_freeder(char **env, char** args, int env_size)
{
    for(int i = 0; i < env_size; i++)
    {
        if (env[i])    
            free(env[i]);
    }
    free(args[0]);
    free(args[1]);
    free(args);
    free(env);
}

void    Post::fill_response(t_client *client, int code, std::string status_line, bool write_it)
{
    t_request *req = client->request;
    t_response *res = client->response;
    std::map<std::string, std::string>  *request_map = &req->request_map;
    std::string connection = req->get_param("connection"); // used for keep-alive

    std::cout << RED_BOLD << code << " -> " << status_line << WHITE << std::endl;
    res->http_version = HTTP_VERSION;
    res->status_code = std::to_string(code);
    res->status_line = status_line ;
    if (sz(connection)) // type of connection
        res->add("connection", "closed");
    if (write_it)
        res->write_response_in_socketfd(client->fd, true);
}

void Post::client_served(t_client* client)
{
    close(client->request->file);
    if (client->response->is_cgi)
    {
        fill_cgi_env(client);
        serve_cgi(client, convert_cgi_env(client), client->response->cgi_env.size());
        return;
    }
    fill_response(client, 201, "Created!", true);
    std::cout << WHITE << "POST REQUEST IS SERVED" << std::endl;
    client->state = SERVED;
}

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
        client->request->file = open("/tmp/cgi_input", O_CREAT | O_TRUNC | O_WRONLY, 0777);
}

char** Post::convert_cgi_env(t_client* client)
{
    t_response* res = client->response;
    int env_size = res->cgi_env.size();
    char** args = (char**) malloc((env_size + 1) * sizeof(char*));
    args[env_size] = NULL;
    std::map<std::string, std::string>::iterator it = res->cgi_env.begin();
    int i = 0;
    for(; it != res->cgi_env.end(); it++)
    {
        std::string arg = it->first + it->second;
        args[i] = strdup(arg.c_str());
        std::cout << CYAN_BOLD << args[i] << WHITE << std::endl;
        i++;
    }
    return args;
}

void    Post::fill_cgi_env(t_client* client)
{
    client->response->cgi_env["REQUEST_METHOD="] = client->request->method;
    client->response->cgi_env["REDIRECT_STATUS="] = "200";
    client->response->cgi_env["CONTENT_LENGTH="] = intToString(client->request->body_size);
    client->response->cgi_env["SCRIPT_FILENAME="] = client->response->filepath;
    client->response->cgi_env["PATH_INFO="] = client->response->filepath;
    client->response->cgi_env["CONTENT_TYPE="] = client->request->request_map.at("content-type");
    if (!client->request->cookies.empty())
        client->response->cgi_env["HTTP_COOKIE="] = client->request->cookies;
}

void    Post::serve_cgi(t_client* client, char** env, int args_size)
{
    int status;
    std::cout << CYAN_BOLD << "HANDLING CGI ..." << std::endl;
    std::cout << "CGI PATH ==> " << WHITE << client->response->cgi_path << std::endl;
    char** args = (char **) malloc (3 * sizeof(char *));
    args[0] = strdup(client->response->cgi_path.c_str());
    args[1] = strdup(client->response->filepath.c_str());
    args[2] = NULL;
    client->response->cgi_pid = fork();
    if (client->response->cgi_pid < 0)
    {
        std::cerr << RED_BOLD << "Forking for CGI failed!" << WHITE << std::endl;
        fill_response(client, 501, "Internal Server Error", true);
        client->state = SERVED;
        return;
    }
    if (!client->response->cgi_pid)
    {
        int cgi_io[2];
        cgi_io[0] = open("/tmp/cgi_input", O_RDONLY);
        cgi_io[1] = open("/tmp/cgi_output", O_CREAT | O_TRUNC | O_WRONLY, 0777);
        if (cgi_io[0] > 0)
            dup2(cgi_io[0], 0);
        if (cgi_io[1] != 1)
            dup2(cgi_io[1], 1);
        if (execve(args[0], args, env))
        {
            std::cerr << RED_BOLD << "EXECVE OR CGI IS NOT WORKING" << WHITE << std::endl;
            exit(-1);
        }
    }
    memory_freeder(env, args, args_size);
    client->response->cgi_running = true;
}

void Post::parse_cgi_output(t_client* client)
{
    int cgi_out = open("/tmp/cgi_output", O_RDONLY), rbytes = 0;
    char buff[MAX_BUFFER_SIZE];
    if (cgi_out < 0)
    {
        fill_response(client, 501, "Internal Server Error", true);
        client->state = SERVED;
        return ;
    }
    write(client->fd, "HTTP/1.1 200 OK\r\n", 17);
    while(true)
    {
        rbytes = read(cgi_out, buff, MAX_BUFFER_SIZE);
        if (!rbytes)
            break;
        write(client->fd, buff, rbytes);
    }
    client->state = SERVED;
    std::cout << WHITE << "POST REQUEST IS SERVED" << std::endl;
}

void Post::serve_client(t_client *client)
{
    if (client->request->first_time)
    {
        client->request_time = time(NULL);
        client->response->cgi_running = false;
        client->request->body_size = 0;
        create_file(client);
        if (client->request->file < 0)
        {
            fill_response(client, 501, "Internal Server Error", true);
            client->state = SERVED;
            return;
        }
        client->request->first_time = false;
    }
    else if (time(NULL) - client->request_time > 10)
    {
        fill_response(client, 408, "Request Timeout", true);
        if (client->response->cgi_running)
            kill(client->response->cgi_pid, SIGKILL);
        client->state = SERVED;
        return;
    }
    if (!client->response->cgi_running)
    {
        char buff[MAX_BUFFER_SIZE];
        int rbytes = read(client->fd, buff, MAX_BUFFER_SIZE);
        if (!rbytes)
        {
            client->state = SERVED;
            return;
        }
        if (rbytes > 0)
            client->request->body.append(buff, rbytes);
        if (!client->request->is_chunked)
            parse_non_chunked_body(client);
        else
            parse_chunked_body(client);
    }
    int status;
    if (client->response->cgi_running && waitpid(-1, &status, WNOHANG) > 0)
    {
        if ((WIFEXITED(status) && WEXITSTATUS(status) == -1))
        {
            client->state = SERVED;
            fill_response(client, 501, "Internal Server Error", true);
            return;
        }
        parse_cgi_output(client);
    }
}

void    Post::parse_non_chunked_body(t_client* client)
{
    t_request* req = client->request;
    int lsize = req->body.size();
    if (req->body_size + lsize < req->content_length)
    {
        write(req->file, req->body.c_str(), lsize);
        req->body_size += lsize;
        req->body.clear();
    }
    else
    {
        write(req->file, req->body.c_str(), req->content_length - req->body_size);
        req->body_size += req->content_length - req->body_size;
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
                client->state = SERVED;
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