#include "../inc/poll.class.hpp"

/**
 * the default multiplexer which is poll
 * this is where all the multiplexing work is being handled
*/


/**
 * function that returns the sockaddr_in struct
 * it initializes it with the usual values and returns it
**/

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
    {
        std::cerr << "SOCKET ERROR" << std::endl;
        exit(1);
    }
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &oplen, sizeof(oplen));
    int bs = bind(fd, (struct sockaddr *)data, data_len);
    if (bs < 0)
    {
        std::cerr << "ERROR IN BIND" << std::endl;
        exit(1);
    }
    int ls = listen(fd, 10);
    if (ls < 0)
    {
        std::cerr << "ERROR IN LISTEN" << std::endl;
        exit(1);
    }
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

t_manager *Poll::get_manager()
{
    t_manager *_manager = new t_manager();
    _manager->handlers.insert(std::make_pair("GET", new Get()));
    _manager->handlers.insert(std::make_pair("POST", new Post()));
    _manager->handlers.insert(std::make_pair("DELETE", new Delete()));
    _manager->cwd = getwd(NULL);
    _manager->fds = new struct pollfd[MAX_FDS]();
    int i;

    i = 0;
    std::cout << "NUMBER OF SERVERS => " << sz((*servers)) << std::endl;
    for (; i < sz((*servers)); i++)
    {
        int port = servers->at(i)->server_configs->port;
        _manager->fds[i].fd = getsocketfd(port);
        _manager->fds[i].events = POLLIN;
        _manager->fds[i].revents = 0;
        if (!_manager->add_server(_manager->fds[i].fd, servers->at(i)))
            std::cerr << "[SERVER MAP FAILED]: socket listener has not been added!" << std::endl;
    }
    for (; i < MAX_FDS; i++)
    {
        _manager->fds[i].fd = UNDEFINED;
        _manager->fds[i].events = 0;
        _manager->fds[i].revents = 0;
        _manager->add_slot(i);
    }
    this->manager = _manager;
    return _manager;
}

/**
 * a function that handles the connection for the first time it gets knocking
 * accepts the user, and tells the manager to track it
*/
void Poll::handle_connection(t_manager *manager, SOCKET fd)
{
    t_server *server = manager->get_server(fd);
    PORT port = server->server_configs->port;
    struct sockaddr_in _data;
    bzero(&_data, sizeof(_data));
    socklen_t len = sizeof((_data));
    SOCKET con = accept(fd, (sockaddr *) &_data, &len);
    fcntl(con, F_SETFL, O_NONBLOCK);
	setsockopt(con, SOL_SOCKET, SO_NOSIGPIPE, &_data, len);
    if (con < 0)
    {
        std::cout << "[accept error] - server internal error!" << std::endl;
        std::cout << strerror(errno) << std::endl;
        exit(1);
    }
    if (!manager->add_client(con, server))
        std::cout << "[CLIENT MAP FAILED]: client has not been added successfully!" << std::endl;
    handle_client(manager, con);
    std::cout << "client with socket fd " << con << " has been connected!" << std::endl;
}


/**
 * handles the disconnection, basically does nothing just calls the manager to remove the client from the map
 * used only for clarification seek
*/

void Poll::handle_disconnection(t_manager *manager, SOCKET fd)
{
    std::cout << "Connection was closed" << std::endl;
    manager->remove_client(fd);
    std::cout << "client with socket fd " << fd << " has been disconnected!" << std::endl;
    std::cout << "NUMBER OF FREE PLACES -> " << MAX_FDS - sz(manager->clients_map) << std::endl;
}

/**
 * returns true if we are dealing with a state that requires reading the header
*/
bool is_http_state(CLIENT_STATE state)
{
    return (state == READING_HEADER || state == WAITING);
}

/**
 * returns true if the client is being in a method serving state
*/

bool is_method_handler_state(CLIENT_STATE state)
{
    return (state == SERVING_GET || state == SERVING_POST || state == SERVING_DELETE);
}

void Poll::handle_client(t_manager *manager, SOCKET fd)
{
    t_client *client = manager->get_client(fd);

    if (is_http_state(client->state))
        http_handler->handle_http(client);
    if (is_method_handler_state(client->state))
        method_handlers->at(client->request->method)->serve_client(client);

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
    t_manager *m = get_manager();
    http_handler = new HttpHandler();
    struct pollfd *fds;

    http_handler->set_mimes(mimes);
    http_handler->set_codes(codes);
    int num_sockets;

    std::cout << manager->fds[0].fd << std::endl;
    manager->client_num = 0;
    signal(SIGPIPE, SIG_IGN);
    while (1)
    {
        num_sockets = sz(manager->clients_map) + sz(manager->servers_map);
        int revents = poll(manager->fds, MAX_FDS, -1);
        if (revents == -1)
        {
            std::cerr << RED_BOLD << "[Crash in Poll]: Internal Server Error" << std::endl;
            std::cout << strerror(errno) << std::endl;
            return ;
        }
        for (int i = 0; i < MAX_FDS; i++)
        {
            if (manager->fds[i].fd != -1)
            {
                if (manager->is_listener(manager->fds[i].fd) &&
                    manager->fds[i].revents & POLLIN) // server
                {
                    handle_connection(manager, manager->fds[i].fd); // connection
                    manager->client_num += 1;
                }
                else // client
                {
                    if ((manager->fds[i].revents & POLLIN) ||
                        (manager->fds[i].revents & POLLOUT)) // handling client
                        handle_client(manager, manager->fds[i].fd);
                    else if (manager->fds[i].revents & POLLHUP) // disconnection
                    {
                        handle_disconnection(manager, manager->fds[i].fd);
                        manager->client_num -= 1;
                    }
                }
            }
        }
    }
}