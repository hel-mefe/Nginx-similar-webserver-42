# include "../inc/get.class.hpp"
# include <dirent.h>
# include <unistd.h>

void    Get::write_hex(SOCKET fd, int n)
{
    std::string s_hex = "0123456789abcdef";

    if (n < 16)
    {
        std::cout << WHITE_BOLD << s_hex[n];
        write(fd, &s_hex[n], 1);
    }
    else
    {
        write_hex(fd, n / 16); // get first char
        write_hex(fd, n % 16); // get others recursively
    }
}

void    Get::write_chunk(SOCKET fd, unsigned char *buff, int len)
{
    write_hex(fd, len);
    // std::cout << WHITE_BOLD << "\r\n" << buff << "\r\n" << WHITE << std::endl;
    write(fd, "\r\n", 2);
    if (len)
        write(fd, buff, len);
    write(fd, "\r\n", 2);
}

void    Get::write_schunk(SOCKET fd, std::string &s, int len)
{
    write_hex(fd, len);
    std::cout << YELLOW_BOLD << s << std::endl;
    write(fd, "\r\n", 2);
    if (len)
        write(fd, s.c_str(), len);
    write(fd, "\r\n", 2);  
}

void    Get::serve_cgi(t_client *client)
{
    t_response *res;
    t_request *req;
    unsigned char buff[MAX_BUFFER_SIZE];
    int bts;
    int read_fd;
    int chunked_start;

    std::cout << YELLOW_BOLD << "CGI IS BEING SERVED ..." << WHITE << std::endl;
    res = client->response;
    req = client->request;
    read_fd = res->cgi_pipe[0];
    bzero(buff, MAX_BUFFER_SIZE);
    chunked_start = 0;
    bts = read(read_fd, buff, MAX_BUFFER_SIZE);
    bts = bts < 0 ? 0 : bts;
    if (!res->cgi_rn_found)
    {
        res->cgi_rn_found = true;
        chunked_start = get_rn_endpos(buff, MAX_BUFFER_SIZE);
    }
    std::cout << WHITE_BOLD << "BUFFER => " << buff << WHITE << std::endl;
    int i = 0;
    for (; i < chunked_start; i++)
        write(client->fd, &buff[i], 1);
    write_chunk(client->fd, buff + i, bts - i);
    if (!bts)
    {
        close(res->cgi_pipe[0]);
        res->cgi_path[0] = UNDEFINED;
        res->cgi_path[1] = UNDEFINED;
        client->state = SERVED;
    }
}

void    print_fd(int fd)
{
    unsigned char buff[MAX_BUFFER_SIZE];

    // std::cout << GREEN_BOLD << "PRINTING FD " << std::endl;
    read(fd, buff, MAX_BUFFER_SIZE);
    // std::cout << RED_BOLD << buff << WHITE << std::endl;
    exit(0);
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
    for (auto x: m)
    {
        std::string s = x.first + "=" + x.second;
        env[i++] = strdup(s.c_str());
        std::cout << "ENV => " << i - 1 << " - " << env[i - 1] << std::endl;
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
    std::cout << CYAN_BOLD << "HANDLING CGI IN GET ...." << std::endl;
    cgi_env["SCRIPT_FILENAME"] = res->filepath;
    // cgi_env["SCRIPT_NAME"] = client->server->server_configs->extension_cgi[res->extension];
    cgi_env["REQUEST_METHOD"] = "GET";
    cgi_env["REDIRECT_STATUS"] = "200";
    cgi_env["GATEWAY_INTERFACE"] = "CGI/1.1";
    cgi_env["HTTP_PROTOCOL"] = "HTTP/1.1";
    cgi_env["HTTP_COOKIE"] = req->cookies;
    std::cout << CYAN_BOLD << "END INIT MAP ...." << std::endl;
    env = convert_env_map(cgi_env);
    std::cout << CYAN_BOLD << "AFTER CONVERT ...." << std::endl;
    args = (char **) malloc (3 * sizeof(char *));
    std::cout << "CGI PATH ==> " << res->cgi_path << std::endl;
    args[0] = strdup(res->cgi_path.c_str());
    args[1] = strdup(res->filepath.c_str()) ;
    args[2] = NULL;
    cgi_path = args[0];
    // std::cout << WHITE_BOLD << "FILE_PATH => " << args[0] << std::endl;
    // std::cout << "CGI_PATH => " << cgi_path << std::endl;

    if (res->cgi_pipe[0] == UNDEFINED) // pipe not piped 
    {
        if (!pipe(res->cgi_pipe))
            std::cout << GREEN_BOLD << "PIPED!" << std::endl;
    }
    if (fork() == 0)
    {
        dup2(res->cgi_pipe[0], 0);
        dup2(res->cgi_pipe[1], 1);
        close(res->cgi_pipe[0]);
        close(res->cgi_pipe[1]);
        if (execve(cgi_path, args, env))
            std::cout << "FAILED! - " << strerror(errno) << std::endl;
        std::cout << "EXECVE NOT WORKING" << std::endl;
    }
    std::cout << "WILL FREE ARGUMENTS NOW ..." << std::endl;
    free(args[0]);
    free(args[1]);
    free(args);
    close(res->cgi_pipe[1]);
    std::cout << "WAITING FOR PROCESS ..." << std::endl;
    waitpid(-1, &status, WNOHANG);
    // print_fd(res->cgi_pipe[0]);
    serve_cgi(client);
}

/** 
 * handling static files file.html, file.txt, file.jpeg ... etc 
 * using chunked for transfer-encoding
 **/
void    Get::handle_static_file(t_client *client)
{
    SOCKET                  sockfd;
    unsigned char           buff[MAX_BUFFER_SIZE];
    int                     bts;
    t_request               *req;
    t_response              *res;

    req = client->request;
    res = client->response;
    sockfd = client->fd;
    bzero(buff, MAX_BUFFER_SIZE);
    bts = read(res->fd, buff, MAX_BUFFER_SIZE);
    if (bts != -1)
        write_chunk(sockfd, buff, bts);
    client->state = (bts ? client->state : SERVED);
    if (client->state == SERVED)
        std::cout << PURPLE_BOLD << "*** FROM GET CLIENT SERVED ***" << WHITE << std::endl ;
}

/*** start listing all the directories ***/
void    Get::list_directories(t_client *client)
{
    t_request               *req;
    t_response              *res;
    DIR                     *dirstream;
    std::string             fullpath;
    struct dirent           *d;

    req = client->request;
    res = client->response;
    fullpath = getwd(NULL);
    fullpath += res->rootfilepath;
    dirstream = opendir(fullpath.c_str());
    if (!dirstream)
    {
        std::cout << "DIR STREAM IS NULL ==> " << fullpath << std::endl;
        client->state = SERVED;
        return ;
    }
    std::cout << "****** DIRECTORY LISTING IS STARTING NOW ... *********" << std::endl;
    while (1)
    {
        d = readdir(dirstream);
        if (!d)
            break ;
        std::string dirname = d->d_name;
        std::string abspath = getwd(NULL);
        abspath = abspath + "/" + dirname;
        res->dir_link.push_back(std::make_pair(dirname, abspath));
        std::cout << dirname << std::endl;
        std::cout << abspath << std::endl;
    }
    closedir(dirstream);
    std::cout << "****** DIRECTORY LISTING HAS BEEN ENDED ... *********" << std::endl;
}

/**
 * <ul>
 * <a href="$dir_link"></a>
 * </ul>
*/
void    Get::serve_directory_listing(t_client *client)
{
    t_request *req;
    t_response *res;
    std::list<std::string>   keep;
    char        dir[1025];

    bzero(dir, 1025);
    req = client->request;
    res = client->response;
    list_directories(client);
    std::string ultag = "<ul>";
    std::string uletag = "</ul>";
    std::cout << RED_BOLD << " ****** PRINTING THE DIRECTORY LISTING RESPONSE *******" << std::endl;
    write_schunk(client->fd, ultag, sz(ultag));
    for (auto dirlink: res->dir_link)
    {
        std::cout << RED_BOLD << "GOT INTO THE WHILE" << std::endl;
        std::string name = dirlink.first;
        std::string link = dirlink.second;
        std::string tag = "<li><a href=\"";
        tag += req->path + name + "\" /> " + name + "</li>" ;
        keep.push_back(tag);
        std::cout << "THE FULL TAG IS => " << tag << std::endl;
    }
    std::cout << "ENDED THE WHILE" << std::endl;
    std::string endline = "";
    for (auto k: keep)
        write_schunk(client->fd, k, sz(k));
    write_schunk(client->fd, uletag, sz(uletag));
    write_schunk(client->fd, endline, 0);
    client->state = SERVED;
    std::cout << RED_BOLD << " ****** END PRINTING THE DIRECTORY LISTING RESPONSE *******" << std::endl;

}
/*** end listing all the directories ***/

void    Get::handle_directory_listing(t_client *client)
{
    t_response  *res;
    t_request   *req;
    std::string html;
    std::string content_length;

    res = client->response;
    req = client->request;
    list_directories(client);
    html = "<html><head><title>Index of</title></head><body><br /><h2>Index of</h2><hr /><ul>";
    for (auto d: res->dir_link)
    {
        std::string dir = d.first;
        std::string link = d.second;
        std::string tag = "<li><a href=\"" + dir + "\" />" + dir + "</li>";
        html += tag;
    }
    html += "</ul></body></html>\r\n";
    content_length = "content_length: " + std::to_string(sz(html)) + "\r\n\r\n";
    send(client->fd, content_length.c_str(), sz(content_length), 0);
    send(client->fd, html.c_str(), sz(html), 0);
    client->state = SERVED;
    // write_schunk(client->fd, html, sz(html), 0);
    // send(client->fd, "0\r\n\r\n", 5, 0);
    std::cout << "-- end of directory listing -- " << std::endl;
}

void    Get::serve_client(t_client *client)
{
    SOCKET      sockfd;
    int         bts;
    t_response  *res;
    unsigned char        buff[MAX_BUFFER_SIZE];

    res = client->response;
    sockfd = client->fd;
    if (res->is_directory_listing) // dealing with directory listing
        handle_directory_listing(client);
    else
    {
        if (res->is_cgi)
            handle_cgi(client);
        else
            handle_static_file(client);
    }
}