# include "../includes/get.class.hpp"
# include <dirent.h>
# include <unistd.h>
# include <sys/stat.h>
#include <signal.h>

long    get_file_size(const char *filename)
{
    struct stat file_status;

    if (stat(filename, &file_status) < 0)
        return -1;
    return (file_status.st_size);
}

void    Get::handle_static_file(t_client *client)
{
    int                     bts;
    t_response              *res;
    std::string             ress;


    res = client->response;
    bzero(res->buffer, MAX_BUFFER_SIZE);
    bts = read(res->fd, res->buffer, MAX_BUFFER_SIZE);
    if (bts > 0)
    {
        if (send(client->fd, res->buffer, bts, 0) == -1)
            std::cout << RED_BOLD << "SEND ERROR -> " << strerror(errno) << std::endl;
    }
    else if (bts <= 0)
        client->state = SERVED;
}


/**
 * one work this function does is listing the directories in the response struct
 * the dir_link list is mainly kept for listing the directories
*/

void    Get::list_directories(t_client *client)
{
    t_response              *res;
    DIR                     *dirstream;
    std::string             fullpath;
    struct dirent           *d;

    res = client->response;
    fullpath = client->cwd;
    fullpath += res->rootfilepath;
    dirstream = opendir(fullpath.c_str());
    if (!dirstream)
    {
        client->state = SERVED;
        return ;
    }
    while (1)
    {
        d = readdir(dirstream);
        if (!d)
            break ;
        std::string dirname = d->d_name;
        std::string abspath = client->cwd;
        abspath = abspath + "/" + res->rootfilepath + "/" + dirname;
        res->dir_link.push_back(std::make_pair(dirname, abspath));
    }
    closedir(dirstream);
}

/***
 * the main function for directory listing
 * does depend on the above function to list directories
 * listing directories is served in normal mode not chunked transfer-encoding
*/
void    Get::handle_directory_listing(t_client *client)
{
    t_response  *res;
    std::string html;
    std::string content_length;
    DIR         *D;

    res = client->response;
    list_directories(client);
    html = "<html><head><title>Index of</title></head><body><br /><h2>Index of</h2><hr /><ul>";
    for (std::list<std::pair<std::string, std::string> >::iterator it = res->dir_link.begin(); it != res->dir_link.end(); it++)
    {
        std::string dir = it->first;
        std::string link = it->second;
        D = opendir(link.c_str());
        if (D)
        {
            dir += "/";
            closedir(D);
        }
        std::string tag = "<li><a href=\"" + dir + "\" />" + dir + "</li>";
        html += tag;
    }
    html += "</ul></body></html>\r\n";
    content_length = "content_length: " + std::to_string(sz(html)) + "\r\n\r\n";
    send(client->fd, content_length.c_str(), sz(content_length), 0);
    send(client->fd, html.c_str(), sz(html), 0);
    client->state = SERVED;
    client->request_time = time(NULL);
}


/***
 * main function for serving get request
 * this function is the main function for GET
*/
void    Get::serve_client(t_client *client)
{
    t_request* req = client->request;
    t_response* res = client->response;

    if (req->first_time)
    {
        client->request_time = time(NULL);
        req->first_time = false;
    }
    else if (res->is_cgi && time(NULL) - client->request_time > 30)
    {
        fill_response(client, 408, "Request Timeout", true);
        if (res->cgi_running)
            kill(res->cgi_pid, SIGKILL);
        client->state = SERVED;
        return;
    }
    if (res->is_directory_listing) // dealing with directory listing
        handle_directory_listing(client);
    else
    {
        if (res->is_cgi)
        {
            if (!res->cgi_running)
            {
                fill_cgi_env(client);
                serve_cgi(client, convert_cgi_env(client), res->cgi_env.size());
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
        else
            handle_static_file(client);
    }
}