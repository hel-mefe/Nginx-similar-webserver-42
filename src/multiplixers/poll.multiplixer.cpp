#include "../includes/poll.class.hpp"

/**
 * the default multiplexer which is poll
 * this is where all the multiplexing work is being handled
*/

/**
 * function that returns the sockaddr_in struct
 * it initializes it with the usual values and returns it
**/

void    write_error(const std::string err_msg)
{
    std::cout << CYAN_BOLD << "[webserv42]: " << err_msg << std::endl;
    exit(1);
}

struct sockaddr_in *getsocketdata(PORT port)
{
    struct sockaddr_in *data;

    data = new struct sockaddr_in();
    bzero(data, sizeof(*data));
    data->sin_port = htons(port);
    data->sin_addr.s_addr = INADDR_ANY;
    data->sin_family = AF_INET;
    return data;
}

/**
 * function that returns a listener socket fd for the server
 * it takes a port and binds the socket to that port
**/

SOCKET getsocketfd(int port)
{
    struct sockaddr_in *data;
    socklen_t data_len;
    socklen_t oplen = 1;

    data = getsocketdata(port);
    data_len = sizeof(*data);
    SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        write_error("Internal server socket error") ;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &oplen, sizeof(oplen));
    int bs = bind(fd, (struct sockaddr *)data, data_len);
    if (bs < 0)
        write_error("Internal server socket bind error");
    int ls = listen(fd, 10);
    if (ls < 0)
        write_error("Internal server socket listen error");
    return fd;
}

/**
 * initializes the manager and returns it
 * a manager is more like the analogy of someone who's tracking all the work
 * gives orders depending on each situation, if a request has been received
 * then it creates a new client and adds it to a data structure (std::map)
 * also if a connection has been closed it removes that client
 * in summary, the manager is responsible for all of these stuff
**/

void    Poll::set_manager()
{
    t_manager *_manager = new t_manager();

    _manager->handlers.insert(std::make_pair("GET", new Get()));
    _manager->handlers.insert(std::make_pair("POST", new Post()));
    _manager->handlers.insert(std::make_pair("DELETE", new Put()));
    _manager->handlers.insert(std::make_pair("OPTIONS", new Options()));
    _manager->handlers.insert(std::make_pair("PUT", new Options()));
    char *wd = getwd(NULL);
    if (wd)
    {
        _manager->cwd = getwd(NULL);
        free(wd);
    }
    _manager->fds = new struct pollfd[MAX_FDS]();

    if (!sz(_manager->cwd))
        write_error("Internal server getcwd() error");
    int i = 0;
    for (; i < sz((*servers)); i++)
    {
        int port = servers->at(i)->server_configs->port;
        _manager->fds[i].fd = getsocketfd(port);
        _manager->fds[i].events = POLLIN;
        _manager->fds[i].revents = 0;
        if (!_manager->add_server(_manager->fds[i].fd, servers->at(i)))
            write_error("Internal server error related to server management");
    }
    for (; i < MAX_FDS; i++)
    {
        _manager->fds[i].fd = UNDEFINED;
        _manager->fds[i].events = 0;
        _manager->fds[i].revents = 0;
        _manager->add_slot(i);
    }
    this->manager = _manager;
}

/**
 * a function that handles the connection for the first time it gets knocking
 * accepts the user, and tells the manager to track it
*/

void Poll::handle_connection(t_manager *manager, SOCKET fd)
{
    t_server *server = manager->get_server(fd);
    struct sockaddr_in _data;

    bzero(&_data, sizeof(_data));
    socklen_t len = sizeof((_data));
    SOCKET con = accept(fd, (sockaddr *) &_data, &len);
    fcntl(con, F_SETFL, O_NONBLOCK);
    if (con < 0)
        write_error("Internal server socket accept error");
    if (!this->manager->add_client(con, server))
        write_error("Internal server client error");
    std::cout << GREEN_BOLD << "[" << time(NULL) << "]: " << WHITE_BOLD << "client with socket " << con << " has been connected" << std::endl;
    // handle_client(manager, con);
}


/**
 * handles the disconnection, basically does nothing just calls the manager to remove the client from the map
 * used only for clarification seek
*/

void Poll::handle_disconnection(t_manager *manager, SOCKET fd)
{
    (void)manager;
    std::cout << YELLOW_BOLD << "[" << time(NULL) << "]: " << WHITE_BOLD << "client with socket " << fd << " has been disconnected" << std::endl;
    if (!this->manager->remove_client(fd))
        std::cout << fd << " HAS NOT BEEN REMOVED!" << std::endl;
}

void Poll::handle_client(t_manager *manager, SOCKET fd)
{
    t_client *client = manager->get_client(fd);
    int status;
    int rt = waitpid(-1, &status, WNOHANG);
    if (rt > 0 && WIFEXITED(status))
        manager->ex_childs.insert(std::pair<int,int>(rt, WEXITSTATUS(status)));
    if (client->state == SERVING_CGI)
        handle_cgi(client);
    if (IS_HTTP_STATE(client->state) || client->state == WAITING)
        http_handler->handle_http(client);
    else
    {
        if (IS_METHOD_STATE(client->state) && IN_MAP(client->request->request_map, client->request->method))
            manager->handlers[client->request->method]->serve_client(client);
        else
            client->state = SERVED;
    }

    if (client->state == SERVED)
        handle_disconnection(manager, fd);
}

/**
 * main multiplexing method that keeps track of all the work
 * depends on the following 3 functions:
 *   - handle_connection for accepting connection at the first time
 *   - handle_client for serving the client
 *   - handle_disconnection for removing the client from the map of clients
*/

void Poll::multiplex()
{
    int num_sockets;
    set_manager();
    http_handler = new HttpHandler();

    http_handler->set_mimes(mimes);
    http_handler->set_codes(codes);
    signal(SIGPIPE, SIG_IGN);
    std::cout << "POLL IS RUNNING ..." << std::endl;
    while (1)
    {
        num_sockets = sz(manager->clients_map) + sz(manager->servers_map);
        num_sockets = MAX_FDS;
        int revents = poll(manager->fds, num_sockets, -1);
        if (revents == -1)
            write_error("Internal server multiplexer error, poll returned -1 in revents");
        for (int i = 0; i < num_sockets; i++)
        {
            if (manager->fds[i].fd != -1)
            {
                if (manager->is_listener(manager->fds[i].fd) &&
                    manager->fds[i].revents & POLLIN) // server
                {
                    handle_connection(this->manager, manager->fds[i].fd); // connection
                    manager->client_num += 1;
                }
                else // client
                {
                    t_client *client = manager->clients_map[manager->fds[i].fd];
                    if (manager->fds[i].revents & POLLHUP) // disconnection
                    {
                        handle_disconnection(this->manager, manager->fds[i].fd);
                        manager->client_num -= 1;
                    }
                    else if ((manager->fds[i].revents & POLLIN) ||
                        (manager->fds[i].revents & POLLOUT)) // handling client
                        {
                            if (((IS_HTTP_STATE(client->state) || client->request->method == "POST" || client->request->method == "PUT") && manager->fds[i].revents & POLLIN) || \
                                ((client->request->method == "GET" || client->request->method == "DELETE" || client->request->method == "OPTIONS" || client->state == WAITING) \
                                && manager->fds[i].revents & POLLOUT))
                                    handle_client(this->manager, manager->fds[i].fd);
                        }
                }
            }
        }
    }
}