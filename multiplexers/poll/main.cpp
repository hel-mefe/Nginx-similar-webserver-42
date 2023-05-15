# include "socket.hpp"

// std::vector<int> ports;

// void    fill_ports()
// {
//     int port = 9090;
//     for (int i= 0 ; i < 5; i++)
//         ports.push_back(port + i);
// }

// struct sockaddr_in *getsocketdata(PORT port)
// {
//     struct sockaddr_in *data;

//     data = new struct sockaddr_in();
//     bzero(data, sizeof(*data));
//     data->sin_port = htons(port);
//     data->sin_addr.s_addr = INADDR_ANY;
//     data->sin_family = AF_INET;
//     return data ;
// }

// SOCKET getsocketfd(int port)
// {
//     struct sockaddr_in  *data;
//     socklen_t           data_len;

//     data = getsocketdata(port);
//     data_len = sizeof(*data);
//     SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
//     fcntl(fd, F_SETFD, O_NONBLOCK);
//     int bs = bind(fd, (struct sockaddr *)data, data_len);
//     if (bs < 0)
//     {
//         std::cerr << "ERROR IN BIND" << std::endl;
//         exit(1) ;
//     }
//     int ls = listen(fd, MAX_CLIENTS);
//     if (ls < 0)
//     {
//         std::cerr << "ERROR IN LISTEN" << std::endl;
//         exit(1) ; 
//     }
//     return fd;
// }

// t_manager *get_manager()
// {
//     fill_ports();
//     t_manager *manager = new t_manager();
//     manager->handlers.insert(std::make_pair("GET", new Get()));
//     manager->fds = new struct pollfd[MAX_FDS];
//     int i;

//     i = 0;
//     for (; i < sz(ports); i++)
//     {
//         manager->fds[i].fd = getsocketfd(ports[i]);
//         manager->sockets_map.insert(std::make_pair(manager->fds[i].fd, new t_socket(manager->fds[i].fd, ports[i], true, i)));
//         manager->fds[i].events = POLLIN;
//     }
//     for (; i < MAX_FDS; i++)
//         manager->add_slot(i);
//     return manager;
// }

// void    handle_connection(t_manager *manager, SOCKET fd)
// {
//     PORT port = manager->get_socket_data(fd)->port;
//     struct sockaddr_in *data = getsocketdata(port);
//     socklen_t len = sizeof(*data);
//     std::cout << "Connection will get accepted" << std::endl;
//     SOCKET con = accept(fd, (sockaddr *)data, &len);
//     fcntl(con, F_SETFL, O_NONBLOCK);
//     if (con < 0)
//     {
//         std::cout << "Connection error" << std::endl;
//         exit(1);
//     }
//     std::cout << "Connection has been accepted -> " << con << std::endl;
//     manager->add_socket(con, UNDEFINED_PORT, false);
//     delete data;
// }

// bool is_connection_closed(SOCKET fd)
// {
//     char c;

//     return recv(fd, &c, 0, 0) == 0;
// }

// void    handle_client(t_manager *manager, SOCKET fd)
// {
//     t_socket *data = manager->get_socket_data(fd);
//     t_client *client = data->data;
//     if (client->state == WAITING)
//         client->state = READING_HEADER;
//     if (client->state == READING_HEADER)
//         read_header(client);
//     std::string method = client->request->method;
//     // else if (client->state == SERVING_GET)
//     //     get_method(client);
//     // else if (client->state == SERVING_DELETE)
//     //     delete_method(client);
//     // else if (client->state == SERVING_POST)
//     //     post_method(client);
//     std::cout << "METHOD -> " << method << std::endl;
//     if (client->state == SERVING_GET || client->state == SERVING_POST || client->state == SERVING_DELETE)
//         manager->handlers[method]->serve_client(client);
//     if (is_connection_closed(fd))
//         client->state = CONNECTION_CLOSED;
//     if (client->state == CONNECTION_CLOSED)
//     {
//         manager->delete_socket(fd);
//         std::cout << "Connection was closed" << std::endl;
//     }
//     else if (client->state == SERVED)
//     {
//         std::cout << "Client was served succesfully!" << std::endl;
//     }
// }

// int main(void)
// {
//     t_manager *manager = get_manager();
//     struct pollfd *fds;
//     int             num_sockets;

//     while (1)
//     {
//         fds = manager->clone_fds();
//         num_sockets = sz(manager->sockets_map);
//         int revents = poll(manager->fds, MAX_FDS, -1);    
//         for (int i = 0; i < MAX_FDS; i++)
//         {
//             if (manager->is_listener(manager->fds[i].fd))
//             {
//                 if (manager->fds[i].revents & POLLIN)
//                     handle_connection(manager, manager->fds[i].fd);
//             }
//             else if ((manager->fds[i].revents & POLLIN) || (manager->fds[i].revents & POLLOUT))
//                 handle_client(manager, manager->fds[i].fd);
//         }

//     }
// }