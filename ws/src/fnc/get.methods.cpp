# include "../inc/get.class.hpp"

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

void    Get::serve_cgi(t_client *client)
{
    t_response *res;
    t_request *req;
    unsigned char buff[MAX_BUFFER_SIZE];
    int bts;
    int read_fd;
    int chunked_start;

    // std::cout << YELLOW_BOLD << "CGI IS BEING SERVED ..." << WHITE << std::endl;
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
    // std::cout << WHITE_BOLD << "BUFFER => " << buff << WHITE << std::endl;
    int i = 0;
    for (; i < chunked_start; i++)
        write(client->fd, &buff[i], 1);
    write_chunk(client->fd, buff + i, bts - i);
    if (!bts)
    {
        close(res->cgi_pipe[0]);
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

void    Get::handle_cgi(t_client *client)
{
    t_request   *req;
    t_response  *res;
    char        **args;
    char        *cgi_path;
    int         status;

    // std::cout << PURPLE_BOLD << "CGI HANDLER IS RUNNING ..." << WHITE << std::endl;
    res = client->response;
    req = client->request;
    args = (char **) malloc (3 * sizeof(char *));
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
        if (execve(cgi_path, args, NULL))
            std::cout << "FAILED! - " << strerror(errno) << std::endl;
        std::cout << "EXECVE NOT WORKING" << std::endl;
    }
    free(args[0]);
    free(args[1]);
    free(args);
    close(res->cgi_pipe[1]);
    // std::cout << "WAITING FOR PROCESS ..." << std::endl;
    waitpid(-1, &status, WNOHANG);
    // print_fd(res->cgi_pipe[0]);
    serve_cgi(client);
}

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
    // std::cout << CYAN_BOLD << "ROOT FILE PATH = " << res->rootfilepath << WHITE << std::endl;
    bzero(buff, MAX_BUFFER_SIZE);
    bts = read(res->fd, buff, MAX_BUFFER_SIZE);
    bts = bts < 0 ? 0 : bts;
    // std::cout << RED_BOLD << bts << " HAS BEEN READ" << std::endl;
    write_chunk(sockfd, buff, bts);
    // std::cout << PURPLE_BOLD << "RESPONSE WRITTEN!" << WHITE << std::endl;
    client->state = (bts ? client->state : SERVED);
    if (client->state == SERVED)
        std::cout << PURPLE_BOLD << "*** FROM GET CLIENT SERVED ***" << WHITE << std::endl ;

}

void    Get::serve_client(t_client *client)
{
    SOCKET      sockfd;
    int         bts;
    t_response  *res;
    unsigned char        buff[MAX_BUFFER_SIZE];

    res = client->response;
    sockfd = client->fd;
    if (res->is_cgi)
        handle_cgi(client);
    else
        handle_static_file(client);
}