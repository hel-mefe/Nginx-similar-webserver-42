#include "../inc/post.class.hpp"

std::string intToString(int num) {
  std::stringstream ss;
  ss << num;
  return ss.str();
}

void Post::client_served(t_client* client)
{
    t_request* req = client->request;
    std::cout << "Serving POST is finished\n";
    client->state = SERVED;
    req->file.close();
    req->body.clear();
    req->file_stat = false;
    req->body_size = 0;
}

Post::Post(void) {}

void    Post::create_file(t_request* req)
{
    if (req->request_map.find("transfer-encoding") == req->request_map.end())
        req->encod_stat = true;
    else
        req->encod_stat = false;
    std::string filename;
    if (!req->filename.empty())
        filename.append(req->filename);
    else
        filename.append("file");
    for (int i = 0; i < INT_MAX; i++)
    {
        std::string file_path("www/");
        file_path.append(filename);
        if (i)
            file_path.append(intToString(i));
        file_path.append(req->extension);
        if (access(file_path.c_str(), F_OK))
        {
            req->file.open(file_path.c_str(), std::ofstream::app);
            break;
        }
        file_path.clear();
    }
    req->file_stat = 1;
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
    t_request* req = client->request;
    char buff[MAX_BUFFER_SIZE];
    int rbytes = read(client->fd, buff, MAX_BUFFER_SIZE);
    if (!rbytes)
    {
        client->state = SERVED;
        return;
    }
    if (rbytes > 0)
        client->request->body.append(buff, rbytes);
    if (!req->file_stat)
        create_file(client->request);
    if (req->encod_stat)
        handle_normal(client);
    else
        handle_chunked(client);
}

void    Post::handle_normal(t_client* client)
{
    t_request* req = client->request;
    int cl = std::atoi(req->request_map.at("content-length").c_str());
    int lsize = req->body.size();
    if (req->body_size + lsize < cl)
    {
        req->file << req->body;
        req->body_size += lsize;
        req->body.clear();
    }
    else
    {
        req->file << req->body.substr(0, cl - req->body_size);
        fill_response(client, 201, "Created!", true);
        client_served(client);
    }
}

void    Post::handle_chunked(t_client* client)
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
            if (!req->hex) 
            {
                client_served(client);
                fill_response(client, 201, "Created!", true);
                req->body.clear();
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
                req->file << req->body;
                req->body.clear();
                req->hex -= lsize;
                req->body_size += lsize;
                break;
            }
            else
            {
                req->file << req->body.substr(0, req->hex);
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