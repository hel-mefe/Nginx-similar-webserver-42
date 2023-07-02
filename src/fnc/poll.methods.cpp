#include "../inc/poll.class.hpp"

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
    int ls = listen(fd, MAX_CLIENTS);
    if (ls < 0)
    {
        std::cerr << "ERROR IN LISTEN" << std::endl;
        exit(1);
    }
    return fd;
}

t_manager *Poll::get_manager()
{
    t_manager *_manager = new t_manager();
    _manager->handlers.insert(std::make_pair("GET", new Get()));
    _manager->handlers.insert(std::make_pair("POST", new Post()));
    _manager->handlers.insert(std::make_pair("DELETE", new Delete()));
    _manager->fds = new struct pollfd[MAX_FDS]();
    int i;

    i = 0;
    std::cout << "NUMBER OF SERVERS => " << sz((*servers)) << std::endl;
    for (; i < sz((*servers)); i++)
    {
        int port = servers->at(i)->server_configs->port;
        _manager->fds[i].fd = getsocketfd(port);
        _manager->fds[i].events = POLLIN;
        if (!_manager->add_server(_manager->fds[i].fd, servers->at(i)))
            std::cout << "[SERVER MAP FAILED]: socket listener has not been added!" << std::endl;
    }
    for (; i < MAX_FDS; i++)
    {
        _manager->fds[i].fd = UNDEFINED;
        _manager->fds[i].events = 0;
        _manager->add_slot(i);
    }
    this->manager = _manager;
    return _manager;
}

void Poll::handle_connection(t_manager *manager, SOCKET fd)
{
    t_server *server = manager->get_server(fd);
    PORT port = server->server_configs->port;
    struct sockaddr_in *data = getsocketdata(port);
    socklen_t len = sizeof(*data);
    SOCKET con = accept(fd, (sockaddr *)data, &len);
    fcntl(con, F_SETFL, O_NONBLOCK);
    if (con < 0)
        exit(1);
    if (!manager->add_client(con, server))
        std::cout << "[CLIENT MAP FAILED]: client has not been added successfully!" << std::endl;
}

bool is_connection_closed(SOCKET fd)
{
    char c;

    return recv(fd, &c, 0, 0) == 0;
}

void Poll::handle_disconnection(t_manager *manager, SOCKET fd)
{
    /*** LOGS HANDLING ****/
    // std::string logline; // [client N] -> [year-month-day] - [time_start, time_end] - [timespent]
    // t_server_configs    *s_configs = manager->clients_map[fd]->server->server_configs;
    // if (s_configs->logsfile_fd != UNDEFINED) // logfile is defined
    // {
    //     std::cout << GREEN_BOLD << "[LOG WRITTEN] -> " << s_configs->logsfile_fd << " - " << s_configs->logsfile << std::endl;
    //     logline = "[client " + std::to_string(manager->client_num) + "] => " + "[2022-21-06] - [10, 20] - [10s]\n" ;
    //     int fd = open(s_configs->logsfile.c_str(), O_CREAT | O_APPEND, O_RDWR);
    //     std::cout << RED_BOLD << write(fd, logline.c_str(), sz(logline)) << std::endl;
    //     std::cout << "[LOGLINE => " << logline << std::endl;
    // }
    // else
    //     std::cout << CYAN_BOLD << "LOGFILE IS UNDEFINED!" << std::endl;
    /***END LOGS HANDLING ***/

    std::cout << "Connection was closed" << std::endl;
    manager->remove_client(fd);
    std::cout << "NUMBER OF FREE PLACES -> " << MAX_FDS - sz(manager->clients_map) << std::endl;
}

bool is_http_state(CLIENT_STATE state)
{
    return (state == READING_HEADER || state == WAITING);
}

bool is_method_handler_state(CLIENT_STATE state)
{
    return (state == SERVING_GET || state == SERVING_POST || state == SERVING_DELETE);
}

void Poll::handle_client(t_manager *manager, SOCKET fd)
{
    t_client *client = manager->get_client(fd);

    if (!client)
    {
        std::cout << "[NO CLIENT]" << std::endl;
        return;
    }
    // if (client->state == WAITING)
    //     client->state = READING_HEADER;
    if (is_http_state(client->state))
    {
        //std::cout << CYAN_BOLD << "Http handler is working ..." << std::endl;
        http_handler->handle_http(client);
    }
    else if (is_method_handler_state(client->state))
        method_handlers->at(client->request->method)->serve_client(client);
    if (client->state == SERVED)
        handle_disconnection(manager, fd);
}

void Poll::multiplex()
{
    t_manager *m = get_manager();
    http_handler = new HttpHandler();
    http_handler->set_mimes(mimes);
    http_handler->set_codes(codes);
    struct pollfd *fds;
    int num_sockets;
    std::cout << manager->fds[0].fd << std::endl;
    manager->client_num = 0;
    signal(SIGPIPE, SIG_IGN);
    while (1)
    {
        num_sockets = sz(manager->clients_map) + sz(manager->servers_map);
        int revents = poll(manager->fds, MAX_FDS, -1);
        if (revents == -1)
            std::cout << strerror(errno) << std::endl;
        for (int i = 0; i < MAX_FDS; i++)
        {
            if (manager->is_listener(manager->fds[i].fd) &&
                manager->fds[i].revents & POLLIN) // connection
            {
                handle_connection(manager, manager->fds[i].fd);
                manager->client_num += 1;
            }
            else if ((manager->fds[i].revents & POLLIN) ||
                     (manager->fds[i].revents & POLLOUT)) // client handling
                handle_client(manager, manager->fds[i].fd);
            else if (manager->fds[i].revents & POLLHUP) // disconnection
                handle_disconnection(manager, manager->fds[i].fd);
        }
    }
}
