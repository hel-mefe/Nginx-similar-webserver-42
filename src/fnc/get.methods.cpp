# include "../inc/get.class.hpp"
# include <dirent.h>
# include <unistd.h>
<<<<<<< HEAD
# include <sys/stat.h>
=======
# include <signal.h>
# include <sys/stat.h>

long    get_file_size(const char *filename)
{
    struct stat file_status;

    if (stat(filename, &file_status) < 0)
        return -1;
    return (file_status.st_size);
}

void    Get::fill_response(t_client *client, int code, std::string status_line, bool write_it)
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
        res->write_response_in_socketfd(client->fd);
}

char** Get::convert_cgi_env(t_client* client)
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

void    Get::fill_cgi_env(t_client* client)
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
}

void    Get::serve_cgi(t_client* client, char** env, int args_size)
{
    int status;
    std::cout << CYAN_BOLD << "... HANDLING CGI ..." << std::endl;
    std::cout << "CGI PATH ==> " << client->response->cgi_path << WHITE << std::endl;
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
        std::cerr << RED_BOLD << "FORKING FOR CGI FAILED!" << WHITE << std::endl;
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
    client->response->cgi_running = true;
}

void Get::parse_cgi_output(t_client* client)
{
    int cgi_out = open(client->request->cgi_out.c_str(), O_RDONLY), rbytes = 0;
    if (cgi_out < 0)
    {
        fill_response(client, 501, "Internal Server Error", true);
        std::cerr << RED_BOLD << "SERVER COUDN'T OPEN CGI OUTPUT FILE!" << WHITE << std::endl;
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
    while(true)
    {
        rbytes = read(cgi_out, buff, MAX_BUFFER_SIZE);
        if (!rbytes)
            break;
        send(client->fd, buff, rbytes, 0);
    }
    client->state = SERVED;
    close(cgi_out);
    std::cout << WHITE << "REQUEST IS SERVED" << std::endl;
}
>>>>>>> atlas

/**
 * sends a hex in the provided socket fd
 * mainly used in chunked transfer-encoding
*/
void    Get::write_hex(SOCKET fd, int n)
{
    std::string s_hex = "0123456789abcdef";

    if (n < 16)
    {
        std::cout << WHITE_BOLD << s_hex[n];
        send(fd, &s_hex[n], 1, 0);
    }
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
    std::cout << YELLOW_BOLD << s << std::endl;
    send(fd, "\r\n", 2, 0);
    if (len)
        send(fd, s.c_str(), len, 0);
    send(fd, "\r\n", 2, 0);  
}

<<<<<<< HEAD
/***
 * 
 * 
*/
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
    bts = recv(read_fd, buff, MAX_BUFFER_SIZE, 0);
    bts = bts < 0 ? 0 : bts;
    if (!res->cgi_rn_found)
    {
        res->cgi_rn_found = true;
        chunked_start = get_rn_endpos(buff, MAX_BUFFER_SIZE);
    }
    std::cout << WHITE_BOLD << "BUFFER => " << buff << WHITE << std::endl;
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

=======
>>>>>>> atlas
void    print_fd(int fd)
{
    unsigned char buff[MAX_BUFFER_SIZE];

    read(fd, buff, MAX_BUFFER_SIZE);
    exit(0);
}

<<<<<<< HEAD
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

=======
/** 
 * handling static files file.html, file.txt, file.jpeg ... etc 
 * using chunked for transfer-encoding
 **/
>>>>>>> atlas
void    Get::handle_static_file(t_client *client)
{
    SOCKET                  sockfd;
    unsigned char           buff[MAX_BUFFER_SIZE];
    int                     bts;
    t_request               *req;
    t_response              *res;

<<<<<<< HEAD
    std::cout << "handling static file is running ..." << std::endl;
    req = client->request;
    res = client->response;
    if (res->is_first_time)
=======
    res->fd = open(res->filepath.c_str(), O_RDONLY);
    std::cout << "handling static file is running ... " << res->filepath << std::endl;
    req = client->request;
    res = client->response;

    if (req->first_time)
>>>>>>> atlas
    {
        long filesize = get_file_size(res->filepath.c_str());
        std::cout << "FILE SIZE => " << filesize << std::endl;
        std::string fs = "content-length: " + std::to_string(filesize) + "\r\n\r\n";
<<<<<<< HEAD
        res->is_first_time = false;
=======
        req->first_time = false;
>>>>>>> atlas
        send(client->fd, fs.c_str(), sz(fs), 0);
    }
    sockfd = client->fd;
    bzero(buff, MAX_BUFFER_SIZE);
    bts = read(res->fd, buff, MAX_BUFFER_SIZE);
    if (send(client->fd, buff, bts, 0) == -1)
    {
        std::cout << "FD => " << client->fd << std::endl;
        std::cout << "FILE FD => " << res->fd << std::endl;
        std::cout << "Bytes read => " << bts << std::endl;
        std::cout << "Buffer => " << buff << std::endl;
        std::cout << CYAN_BOLD << "[NO SEND] ..." << std::endl;
        std::cout << strerror(errno) << std::endl;
    }
    if (bts < MAX_BUFFER_SIZE)
    {
        if (send(client->fd, "\r\n", 2, 0) == -1)
            std::cout << "[NO SEND] ... 2" << std::endl;
        client->state = SERVED;
        client->request_time = time(NULL);
    }
    std::cout << "end handling static file is running ..." << std::endl;
}


/**
 * one work this function does is listing the directories in the response struct
 * the dir_link list is mainly kept for listing the directories
*/

void    Get::list_directories(t_client *client)
{
    t_request               *req;
    t_response              *res;
    DIR                     *dirstream;
    char                    buff_path[1024];
    std::string             fullpath;
    struct dirent           *d;

    req = client->request;
    res = client->response;
    bzero(buff_path, 1024);
    getcwd(buff_path, 1024);
    fullpath = buff_path;
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
        std::string abspath = buff_path;
        abspath = abspath + "/" + dirname;
        res->dir_link.push_back(std::make_pair(dirname, abspath));
        std::cout << dirname << std::endl;
        std::cout << abspath << std::endl;
    }
    closedir(dirstream);
    std::cout << "****** DIRECTORY LISTING HAS BEEN ENDED ... *********" << std::endl;
}


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

/***
 * the main function for directory listing
 * does depend on the above function to list directories
 * listing directories is served in normal mode not chunked transfer-encoding
*/
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
    client->state = WAITING;
    client->request_time = time(NULL);
    std::cout << "-- end of directory listing -- " << std::endl;
}


/***
 * main function for serving get request
 * this function is the main function for GET
*/
void    Get::serve_client(t_client *client)
{
<<<<<<< HEAD
    t_response  *res;

    std::cout << "[GET] is running ..." << std::endl; 
    res = client->response;
    if (res->is_directory_listing)
        handle_directory_listing(client);
=======
    SOCKET      sockfd = client->fd;
    t_response  *res = client->response;
    if (client->request->first_time)
    {
        client->request_time = time(NULL);
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
    if (res->is_directory_listing) // dealing with directory listing
        serve_directory_listing(client);
>>>>>>> atlas
    else
    {
        if (res->is_cgi)
        {
            if (!res->cgi_running)
            {
                fill_cgi_env(client);
                serve_cgi(client, convert_cgi_env(client), client->response->cgi_env.size());
            }
            int status;
            int rt = waitpid(-1, &status, WNOHANG);
            if (client->response->cgi_running && rt > 0)
            {
                if (WIFEXITED(status) && WEXITSTATUS(status) == 42)
                {
                    client->state = SERVED;
                    std::cerr << RED_BOLD << "SERVER/CGI FAILED!" << WHITE << std::endl;
                    fill_response(client, 501, "Internal Server Error", true);
                    return;
                }
                if (rt == client->response->cgi_pid)
                    parse_cgi_output(client);
            }
        }
        else
            handle_static_file(client);
    }
    std::cout << "[GET] is finished ..." << std::endl;
}