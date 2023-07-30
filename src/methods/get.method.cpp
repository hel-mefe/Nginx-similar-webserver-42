# include "../includes/get.class.hpp"
# include <dirent.h>
# include <unistd.h>
# include <sys/stat.h>
#include <signal.h>

std::string get_hex_as_string(int bts, std::string res)
{
    std::string hex = "0123456789abcdef";
    if (bts < 16)
    {
        res += hex[bts];
        return (res) ;
    }
    res = get_hex_as_string(bts / 16, res);
    res = get_hex_as_string(bts % 16, res);
    return (res) ;
}

void    Get::serve_by_chunked(t_client *client)
{
    t_response  *res;
    std::string resp;
    std::string hex;
    int         bts;

    res = client->response;
    char buffer[MAX_BUFFER_SIZE];
    bzero(buffer, MAX_BUFFER_SIZE);
    bts = read(res->fd, buffer, MAX_BUFFER_SIZE);
    if (bts > 0)
    {
        hex = get_hex_as_string(bts, hex);
        std::cout << "HEX -> " << hex << std::endl;
        resp = hex + "\r\n";
        resp.append(buffer, bts);
        resp += "\r\n";
        if (send(client->fd, resp.c_str(), sz(resp), 0) == -1)
            client->state = SERVED;
    }
    else if (bts <= 0)
    {
        send(client->fd, "0\r\n\r\n", 5, 0);
        close(res->fd);
        client->state = SERVED;
    }
}

void    Get::serve_by_content_length(t_client *client)
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
            client->state = SERVED ;
    }
    else if (bts <= 0)
        client->state = SERVED;

}

void    Get::handle_static_file(t_client *client)
{
    t_response              *res = client->response;
    t_request               *req = client->request;

    if (req->first_time)
    {
        res->fd = open(res->filepath.c_str(), O_RDONLY);
        if (res->fd < 0)
        {
            std::cerr << RED_BOLD << "[error][io]: failed!" << WHITE << std::endl;
            fill_response(client, 501, "Internal Server Error", true);
            client->state = SERVED;
            return;
        }
        req->first_time = false;
    }
    if (res->is_chunked)
        serve_by_chunked(client);
    else
        serve_by_content_length(client);
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
    std::string full_html_response;
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
    full_html_response = content_length + html;

    /** send is protected because in all cases the client state will be set to SERVED **/
    send(client->fd, full_html_response.c_str(), sz(full_html_response), 0);
    client->state = SERVED;
    client->request_time = time(NULL);
}


/***
 * main function for serving get request
 * this function is the main function for GET
*/
void    Get::serve_client(t_client *client)
{
    t_response* res = client->response;

    if (res->is_directory_listing) // dealing with directory listing
        handle_directory_listing(client);
    else
        handle_static_file(client);
}