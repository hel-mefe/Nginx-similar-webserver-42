# include "../includes/get.class.hpp"
# include <dirent.h>
# include <unistd.h>
# include <sys/stat.h>

/**
 * sends a hex in the provided socket fd
 * mainly used in chunked transfer-encoding
*/
void    Get::write_hex(SOCKET fd, int n)
{
    std::string s_hex = "0123456789abcdef";

    if (n < 16)
        send(fd, &s_hex[n], 1, 0);
    else
    {
        write_hex(fd, n / 16); // get first char
        write_hex(fd, n % 16); // get others recursively
    }
}

/**
 * sends a full chunk of characters to the provided socket fd
 * hex\r\n msg\r\n
 * note that msg can be of length 0 which means nothing will be sent
 * this work by unsigned char* buff which is perfect for images and videos 
*/
void    Get::write_chunk(SOCKET fd, unsigned char *buff, int len)
{
    write_hex(fd, len);
    send(fd, "\r\n", 2, 0);
    if (len)
        send(fd, buff, len, 0);
    send(fd, "\r\n", 2, 0);
}

/**
 * sends a full chunk of characters to the provided socket fd
 * hex\r\n msg\r\n
 * note that msg can be of length 0 which means nothing will be sent
 * the difference between it and the above function this deals with a string
 * the above one deals with an unsigned char *buff
*/
void    Get::write_schunk(SOCKET fd, std::string &s, int len)
{
    write_hex(fd, len);
    send(fd, "\r\n", 2, 0);
    if (len)
        send(fd, s.c_str(), len, 0);
    send(fd, "\r\n", 2, 0);  
}

/***
 * 
 * 
*/
void    Get::serve_cgi(t_client *client)
{
    t_response *res;
    unsigned char buff[MAX_BUFFER_SIZE];
    int bts;
    int read_fd;
    int chunked_start;

    res = client->response;
    read_fd = res->cgi_pipe[0];
    bzero(buff, MAX_BUFFER_SIZE);
    chunked_start = 0;
    bts = recv(read_fd, buff, MAX_BUFFER_SIZE, 0);
    bts = bts < 0 ? 0 : bts;
    if (!res->cgi_rn_found)
    {
        res->cgi_rn_found = true;
        chunked_start = get_rn_endpos(buff, MAX_BUFFER_SIZE);
    }
    int i = 0;
    for (; i < chunked_start; i++)
        send(client->fd, &buff[i], 1, 0);
    write_chunk(client->fd, buff + i, bts - i);
    if (!bts)
    {
        close(res->cgi_pipe[0]);
        res->cgi_path[0] = UNDEFINED;
        res->cgi_path[1] = UNDEFINED;
        client->state = SERVED;
    }
}

char    **Get::convert_env_map(std::map<std::string, std::string> &m)
{
    int env_size = sz(m);
    int i;
    char    **env;

    env = (char **)malloc((env_size + 1) * sizeof(char *));
    env[env_size] = nullptr;
    if (!env)
        return (nullptr) ;
    i = 0;
    for (std::map<std::string, std::string>::iterator it = m.begin(); it != m.end(); it++)
    {
        std::string s = it->first + "=" + it->second;
        env[i++] = strdup(s.c_str());
    }
    return env;
}

void    Get::handle_cgi(t_client *client)
{
    t_request   *req;
    t_response  *res;
    char        **args;
    char        *cgi_path;
    char        **env;
    int         status;
    std::map<std::string, std::string>  cgi_env;

    res = client->response;
    req = client->request;
    cgi_env["SCRIPT_FILENAME"] = res->filepath;
    cgi_env["REQUEST_METHOD"] = "GET";
    cgi_env["REDIRECT_STATUS"] = "200";
    cgi_env["GATEWAY_INTERFACE"] = "CGI/1.1";
    cgi_env["HTTP_PROTOCOL"] = "HTTP/1.1";
    cgi_env["HTTP_COOKIE"] = req->cookies;
    env = convert_env_map(cgi_env);
    args = (char **) malloc (3 * sizeof(char *));
    args[0] = strdup(res->cgi_path.c_str());
    args[1] = strdup(res->filepath.c_str()) ;
    args[2] = NULL;
    cgi_path = args[0];
    if (res->cgi_pipe[0] == UNDEFINED)
    {
        if (!pipe(res->cgi_pipe))
        {
            std::cerr << RED_BOLD << "[Webserv42]: Internal server pipe error" << std::endl;
            exit(1);
        }
    }
    if (fork() == 0)
    {
        dup2(res->cgi_pipe[0], 0);
        dup2(res->cgi_pipe[1], 1);
        close(res->cgi_pipe[0]);
        close(res->cgi_pipe[1]);
        execve(cgi_path, args, env);
    }
    free(args[0]);
    free(args[1]);
    free(args);
    close(res->cgi_pipe[1]);
    waitpid(-1, &status, WNOHANG);
    serve_cgi(client);
}

/** 
 * main function for handling any static file
 * static files are file.html, file.txt, file.jpeg ... etc 
 * this functions mainly depends on chunked for transfer-encoding
 **/

long    get_file_size(const char *filename)
{
    struct stat file_status;

    if (stat(filename, &file_status) < 0)
        return -1;
    return (file_status.st_size);
}

void    Get::handle_static_file(t_client *client)
{
    unsigned char           buff[MAX_BUFFER_SIZE];
    int                     bts;
    t_response              *res;

    res = client->response;
    if (res->is_first_time)
    {
        long filesize = get_file_size(res->filepath.c_str());
        std::string fs = "content-length: " + std::to_string(filesize) + "\r\n\r\n";
        res->is_first_time = false;
        send(client->fd, fs.c_str(), sz(fs), 0);
    }
    bzero(buff, MAX_BUFFER_SIZE);
    bts = read(res->fd, buff, MAX_BUFFER_SIZE);
    send(client->fd, buff, bts, 0);
    if (bts < MAX_BUFFER_SIZE)
    {
        send(client->fd, "\r\n", 2, 0);
        client->state = SERVED;
        client->request_time = time(NULL);
    }
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
    t_response  *res;

    res = client->response;
    if (res->is_directory_listing)
        handle_directory_listing(client);
    else
    {
        if (res->is_cgi)
            handle_cgi(client);
        else
            handle_static_file(client);
    }
}