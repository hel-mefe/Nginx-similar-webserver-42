#include "../includes/post.class.hpp"
#include <sys/time.h>
#include <signal.h>

Post::Post(void) {}

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
    client->state = SERVED;
}

void    Post::create_file(t_client* client)
{
    std::string root;
    if (client->response->is_cgi)
        root = "/tmp/cgi_in";
    else
    {
        root = client->cwd +  client->response->rootfilepath;
        if (!client->request->filename.empty())
            root.append(client->request->filename);
        else
            root.append("Uploaded_file");
    }
    for (int i = 0; i < INT_MAX; i++)
    {
        std::string file_path(root);
        if (i)
            file_path.append(intToString(i));
        if (!client->response->is_cgi)
            file_path.append(client->request->extension);
        if (access(file_path.c_str(), F_OK))
        {
            client->request->file = open(file_path.c_str(), O_CREAT | O_APPEND | O_RDWR, 0777);
            if (client->response->is_cgi)
                client->request->cgi_in = file_path;
            break;
        }
    }
}

void Post::serve_client(t_client *client)
{
    t_request* req = client->request;
    t_response* res = client->response;

    if (req->first_time)
    {
        client->request_time = time(NULL);
        res->cgi_running = false;
        req->body_size = 0;
        res->is_cgi = (req->extension == ".php" || req->extension == ".pl" || req->extension == ".py");
        if (res->is_cgi)
            res->cgi_path = res->configs->extension_cgi[req->extension];
        create_file(client);
        if (req->file < 0)
        {
            std::cerr << RED_BOLD << "SERVER COUDN'T OPEN UPLOAD FILE!" << WHITE << std::endl;
            fill_response(client, 501, "Internal Server Error", true);
            client->state = SERVED;
            return;
        }
        req->first_time = false;
    }
    else if (time(NULL) - client->request_time > 30)
    {
        fill_response(client, 408, "Request Timeout", true);
        if (res->cgi_running)
            kill(res->cgi_pid, SIGKILL);
        client->state = SERVED;
        return;
    }
    if (!res->cgi_running)
    {
        char buff[MAX_BUFFER_SIZE];
        int rbytes = recv(client->fd, buff, MAX_BUFFER_SIZE, 0);
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
    if (res->cgi_running)
    {
        int status;
        int rt = waitpid(-1, &status, WNOHANG);
        if (rt == res->cgi_pid)
        {
            if (WIFEXITED(status) && WEXITSTATUS(status) == 42)
            {
                client->state = SERVED;
                std::cerr << RED_BOLD << "SERVER/CGI FAILED!" << WHITE << std::endl;
                fill_response(client, 501, "Internal Server Error", true);
                return;
            }
            parse_cgi_output(client);
        }
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
            if (req->hex + req->body_size > client->response->configs->max_body_size)
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