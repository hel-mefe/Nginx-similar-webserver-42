#include "../includes/kqueue.class.hpp"

/**
 * the default multiplexer which is poll
 * this is where all the multiplexing work is being handled
*/

/**
 * function that returns the sockaddr_in struct
 * it initializes it with the usual values and returns it
**/

void    Kqueue::write_error(const std::string err_msg)
{
    std::cout << CYAN_BOLD << "[webserv42]: " << err_msg << std::endl;
    exit(1);
}

struct sockaddr_in *Kqueue::getsocketdata(PORT port)
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

SOCKET Kqueue::getsocketfd(int port)
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

void    Kqueue::set_manager()
{
    t_kqueue_manager *_manager = new t_kqueue_manager();

    _manager->handlers.insert(std::make_pair("GET", new Get()));
    _manager->handlers.insert(std::make_pair("POST", new Post()));
    _manager->handlers.insert(std::make_pair("DELETE", new Delete()));
    _manager->handlers.insert(std::make_pair("OPTIONS", new Options()));
    _manager->cwd = getwd(NULL);

    if (!sz(_manager->cwd))
        write_error("Internal server getcwd() error");
    int i = 0;
    for (; i < sz((*servers)); i++)
    {
        int port = servers->at(i)->server_configs->port;
        _manager->rEvents[i].ident = getsocketfd(port);
        _manager->add_kqueue_event(_manager->rEvents[i].ident, EVFILT_READ, NULL);
        if (!_manager->add_server(_manager->rEvents[i].ident, servers->at(i)))
            write_error("Internal server error related to server management");
    }
    for (; i < MAX_KQUEUE_FDS; i++)
    {
        _manager->rEvents[i].ident = UNDEFINED;
        _manager->rEvents[i].filter = 0;
        _manager->rEvents[i].flags = 0;
        _manager->rEvents[i].fflags = 0;
        _manager->rEvents[i].data = 0;
        _manager->rEvents[i].udata = NULL;
        _manager->add_slot(i);
    }
    this->manager = _manager;
}

/**
 * a function that handles the connection for the first time it gets knocking
 * accepts the user, and tells the manager to track it
*/

void Kqueue::handle_connection(t_kqueue_manager *manager, SOCKET fd)
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

void Kqueue::handle_disconnection(t_kqueue_manager *manager, SOCKET fd)
{
    (void)manager;
    std::cout << YELLOW_BOLD << "[" << time(NULL) << "]: " << WHITE_BOLD << "client with socket " << fd << " has been disconnected" << std::endl;
    if (!this->manager->remove_client(fd))
        std::cout << fd << " HAS NOT BEEN REMOVED!" << std::endl;
}


bool is_method_handler_state(CLIENT_STATE state)
{
    return (state == SERVING_GET || state == SERVING_POST || state == SERVING_DELETE);
}

void Kqueue::handle_client(t_kqueue_manager *manager, SOCKET fd)
{
    t_client *client = manager->get_client(fd);
    CLIENT_STATE    prev_state = client->state;

    // std::cout << "Handling client " << fd << std::endl;
    if (IS_HTTP_STATE(client->state) || client->state == WAITING || client->state == KEEP_ALIVE)
        http_handler->handle_http(client);
    else if (IS_METHOD_STATE(client->state))
        manager->handlers[client->request->method]->serve_client(client);

    if (client->state == SERVED)
    {
        std::cout << CYAN_BOLD << "SERVED BLOCK -> " << client->request->path << std::endl;
        handle_disconnection(manager, fd);
    }
    else if (prev_state != client->state)
    {
        if (R_STATE(client->state))
        {
            if (W_STATE(prev_state))
            {
                manager->delete_kqueue_event(fd, EVFILT_WRITE);
                manager->add_kqueue_event(fd, EVFILT_READ, client);
            }
        }
        else
        {
            if (R_STATE(prev_state))
            {
                manager->delete_kqueue_event(fd, EVFILT_READ);
                manager->add_kqueue_event(fd, EVFILT_WRITE, client);
            }
        }
    }
}

/**
 * main multiplexing method that keeps track of all the work
 * depends on the following 3 functions:
 *   - handle_connection for accepting connection at the first time
 *   - handle_client for serving the client
 *   - handle_disconnection for removing the client from the map of clients
*/

void Kqueue::multiplex()
{
    set_manager();
    http_handler = new HttpHandler();
    http_handler->set_mimes(mimes);
    http_handler->set_codes(codes);
    signal(SIGPIPE, SIG_IGN);

    SOCKET  fd;
    struct kevent *events = manager->rEvents;
    std::cout << "KQUEUE IS RUNNING ..." << std::endl;
    while (1)
    {
        int revents = kevent(manager->kq, NULL, 0, events, MAX_KQUEUE_FDS, NULL);
        if (revents == -1)
            write_error("Internal server multiplexer error, kqueue returned -1 in revents");
        for (int i = 0; i < revents; i++)
        {
            fd = events[i].ident;
            if (fd != -1)
            {
                if (manager->is_listener(fd)) // server
                {
                    handle_connection(this->manager, fd); // connection
                    manager->client_num += 1;
                }
                else // client
                {
                    // t_client *client = manager->clients_map[fd];
                    if (events[i].flags & EV_EOF) // disconnection
                    {
                        handle_disconnection(this->manager, fd);
                        std::cout << CYAN_BOLD << "EV EOF BLOCK" << std::endl;
                        manager->client_num -= 1;
                    }
                    else
                        handle_client(this->manager, fd);
                }
            }
        }
    }
}