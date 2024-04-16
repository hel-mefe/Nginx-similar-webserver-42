#include "../includes/http_handler.utils.hpp"

bool create_file(t_client* client)
{
    std::string file_path = client->cwd + client->response->root;
    if (access(file_path.c_str(), F_OK))
    {
        fill_response(client, 404, "Not Found", true);
        client->state = SERVED;
        return false;
    }
    if (access(file_path.c_str(), W_OK | X_OK))
    {
        fill_response(client, 403, "Forbidden", true);
        client->state = SERVED;
        return false;
    }
    if (client->request->method == "POST")
    {
        for (int i = 0; i < INT_MAX; i++)
        {
            std::string path = file_path;
            if (client->response->is_cgi)
                path = "/tmp/cgi_in";
            else
                path.append("/uploaded_file");
            if (i)
                path.append(intToString(i));
            if (!client->response->is_cgi)
                path.append(client->request->extension);
            if (access(path.c_str(), F_OK))
            {
                file_path = path;
                break;
            }
        }
    }
    else
    {
        file_path = client->response->filepath;
        if (!access(file_path.c_str(), F_OK))
        {
            client->response->file_exist = true;
            if (access(file_path.c_str(), W_OK))
            {
                fill_response(client, 403, "Forbidden", true);
                client->state = SERVED;
                return false;
            }
        }
    }
    if (!client->response->file_exist)
        client->request->file = open(file_path.c_str(), O_CREAT | O_RDWR, 0777);
    else
        client->request->file = open(file_path.c_str(), O_TRUNC | O_RDWR, 0777);
    if (client->request->file < 0)
    {
        client->state = SERVED;
        fill_response(client, 500, "Internal Server Error", true);
        return false;
    }
    if (client->response->is_cgi)
        client->request->cgi_in = file_path;
    return true;
}

void    upload_file(t_client* client)
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

void upload_finish(t_client* client)
{
    client->request->body.clear();
    close(client->request->file);
    if (!client->response->is_cgi)
    {
        if (!client->response->file_exist)
            fill_response(client, 201, "Created", true);
        else
            fill_response(client, 204, "No content", true);
        client->state = SERVED;
    }
    else
    {
        client->response->cgi_running = true;
        client->request->first_time = true;
    }
}

void   parse_non_chunked_body(t_client* client)
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
        upload_finish(client);
    }
}

void   parse_chunked_body(t_client* client)
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
                upload_finish(client);
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