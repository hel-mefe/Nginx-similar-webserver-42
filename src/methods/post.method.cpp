#include "../includes/post.class.hpp"
#include <sys/time.h>
#include <signal.h>

Post::Post(void) {}

void Post::serve_client(t_client *client)
{
    t_request* req = client->request;
    t_response* res = client->response;

    if (req->first_time)
    {
        client->request_time = time(NULL);
        res->cgi_running = false;
        req->body_size = 0;
        res->is_cgi = IS_CGI_SUPPORTED(req->method);
        if (res->is_cgi)
            res->cgi_path = res->configs->extension_cgi[req->extension];
        if (!create_file(client))
            return;
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
    upload_file(client);
    // if (!res->cgi_running)
    // {
    // }
    // if (res->cgi_running)
    // {
    //     int status;
    //     int rt = waitpid(-1, &status, WNOHANG);
    //     if (rt == res->cgi_pid)
    //     {
    //         if (WIFEXITED(status) && WEXITSTATUS(status) == 42)
    //         {
    //             client->state = SERVED;
    //             std::cerr << RED_BOLD << "SERVER/CGI FAILED!" << WHITE << std::endl;
    //             fill_response(client, 501, "Internal Server Error", true);
    //             return;
    //         }
    //         parse_cgi_output(client);
    //     }
    // }
}

Post::~Post(void) {}