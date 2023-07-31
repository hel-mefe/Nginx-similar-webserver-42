#include "../includes/http_handler.class.hpp"
#include <signal.h>

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

char** convert_cgi_env(t_client* client)
{
    t_response* res = client->response;
    int env_size = res->cgi_env.size();
    char** args = (char**) malloc((env_size + 1) * sizeof(char*));
    std::cout << CYAN_BOLD << "... [HANDLING CGI] ..." << WHITE << std::endl;
    args[env_size] = NULL;
    std::map<std::string, std::string>::iterator it = res->cgi_env.begin();
    int i = 0;
    for(; it != res->cgi_env.end(); it++)
    {
        std::string arg = it->first + it->second;
        args[i] = strdup(arg.c_str());
        std::cout << args[i] << std::endl;
        i++;
    }
    return args;
}

void    fill_cgi_env(t_client* client)
{
    client->response->cgi_env["REQUEST_METHOD="] = client->request->method;
    client->response->cgi_env["REDIRECT_STATUS="] = "200";
    client->response->cgi_env["CONTENT_LENGTH="] = intToString(client->request->body_size);
    client->response->cgi_env["SCRIPT_FILENAME="] = client->response->filepath;
    client->response->cgi_env["PATH_INFO="] = client->response->filepath;
    if (client->request->method == "POST")
        client->response->cgi_env["CONTENT_TYPE="] = client->request->request_map.at("content-type");
    if (!client->request->cookies.empty())
        client->response->cgi_env["HTTP_COOKIE="] = client->request->cookies;
    if (!client->request->queries.empty())
        client->response->cgi_env["QUERY_STRING="] = client->request->queries;
}

void    serve_cgi(t_client* client, char** env, int args_size)
{
    std::cout << "[CGI_PATH]: " << client->response->cgi_path << std::endl;
    char** args = (char **) malloc (3 * sizeof(char *));
    args[0] = strdup(client->response->cgi_path.c_str());
    args[1] = strdup(client->response->filepath.c_str());
    args[2] = NULL;
    for (int i = 0; i < INT_MAX; i++)
    {
        std::string file_path = "/tmp/cgi_out";
        if (i)
            file_path.append(intToString(i));
        if (access(file_path.c_str(), F_OK))
        {
            client->request->cgi_out = file_path;
            break;
        }
    }
    client->response->cgi_pid = fork();
    if (client->response->cgi_pid < 0)
    {
        std::cerr << RED_BOLD << "[error][cgi]: failed!" << WHITE << std::endl;
        fill_response(client, 501, "Internal Server Error", true);
        client->state = SERVED;
        return;
    }
    if (!client->response->cgi_pid)
    {
        int cgi_io[2];
        if (client->request->method == "POST")
        {
            cgi_io[0] = open(client->request->cgi_in.c_str(), O_RDONLY);
            if (cgi_io[0] > 0)
                dup2(cgi_io[0], 0);
            else
                exit(42);
        }
        cgi_io[1] = open(client->request->cgi_out.c_str(), O_CREAT | O_APPEND | O_RDWR, 0777);
        if (cgi_io[1] > 0)
            dup2(cgi_io[1], 1);
        else
            exit(42);
        if (execve(args[0], args, env))
            exit(42);
    }
    memory_freeder(env, args, args_size);
}

void parse_cgi_output(t_client* client)
{
    int cgi_out = open(client->request->cgi_out.c_str(), O_RDONLY), rbytes = 0;
    if (cgi_out < 0)
    {
        fill_response(client, 501, "Internal Server Error", true);
        std::cerr << RED_BOLD <<"[error][cgi]: failed!" << WHITE << std::endl;
        client->state = SERVED;
        return ;
    }
    char buff[MAX_BUFFER_SIZE];
    send(client->fd, "HTTP/1.1", 8, 0);
    rbytes = read(cgi_out, buff, MAX_BUFFER_SIZE);
    if (rbytes)
    {
        std::string str(buff, rbytes);
        size_t epos = str.find("\r\n");
        std::string line(str, 0, epos + 2);
        size_t spos = line.find("Status:");
        if(spos == std::string::npos)
            send(client->fd, " 200 OK\r\n", 9, 0);
        else
        {
            line.erase(0, 7);
            send(client->fd, line.c_str(), line.size(), 0);
            str.erase(0, epos + 2);
        }
        send(client->fd, str.c_str(), str.size(), 0);
    }
    // removed later on
    while(true)
    {
        rbytes = read(cgi_out, buff, MAX_BUFFER_SIZE);
        if (!rbytes)
            break;
        send(client->fd, buff, rbytes, 0);
    }
    if (IN_MAP(client->request->request_map, "connection") && client->request->request_map["connection"] == "keep-alive")
        client->state = KEEP_ALIVE;
    else
        client->state = SERVED;
    close(cgi_out);
}

void    handle_cgi(t_client* client)
{
    t_request* req = client->request;
    t_response* res = client->response;

    if (client->request->first_time)
    {
        client->request_time = time(NULL);
        req->body_size = 0;
        if (req->method == "POST" && !create_file(client))
            return;
        req->first_time = false;
    }
    else if (res->is_cgi && time(NULL) - client->request_time >= client->server->http_configs->max_cgi_timeout)
    {
        fill_response(client, 408, "Request Timeout", true);
        if (res->cgi_running)
            kill(res->cgi_pid, SIGKILL);
        client->state = SERVED;
        return;
    }
    if (!client->response->cgi_running)
    {
        if (client->request->method == "POST")
            upload_file(client);
        else
        {
            client->response->cgi_running = true;
            client->request->first_time = true;
        }
    }
    if (client->request->first_time)
    {
        std::cout << "## FILL CGI START ##\n";
        fill_cgi_env(client);
        serve_cgi(client, convert_cgi_env(client), client->response->cgi_env.size());
        req->first_time = false;
    }
    std::map<int,int>::iterator it = client->ex_childs->find(client->response->cgi_pid);
    if (it != client->ex_childs->end())
    {
        if (it->second == 42)
        {
            client->state = SERVED;
            std::cerr << RED_BOLD << "[error][cgi]: failed!" << WHITE << std::endl;
            fill_response(client, 501, "Internal Server Error", true);
            return;
        }
        client->ex_childs->erase(res->cgi_pid);
        parse_cgi_output(client);
    }
}