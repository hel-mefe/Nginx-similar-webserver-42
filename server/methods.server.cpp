# include "class.server.hpp"

Server::Server() : server_configs(0), http_configs(0), dir_configs(0)
{
    server_configs = new t_server_configs();
    http_configs = new t_http_configs();
    dir_configs = new HashMap<std::string, t_location_configs*>();
}


Server::Server(const Server &s) : server_configs(0), http_configs(0), dir_configs(0)
{
    *this = s;
}

Server& Server::operator=(const Server &s)
{
    return (*this);
}

Server::~Server()
{
    if (server_configs)
        delete server_configs;
    if (http_configs)
        delete http_configs;
    if (dir_configs)
        delete dir_configs;
}


void    Server::set_server_configs(t_server_configs *_server_configs)
{
    if (this->server_configs)
        delete this->server_configs;
    this->server_configs = _server_configs;
}

void    Server::set_location_map(std::string location_name, t_location_configs *configs)
{
    dir_configs->insert(std::make_pair(location_name, configs));
}

// void    Server::print_data()
// {
//     std::cout << "**** SERVER ATTRIBUTES ****" << std::endl;
//     std::cout << "server_name -> " << server_configs->server_name << std::endl;
//     std::cout << "port -> " << server_configs->port << std::endl;
//     std::cout << "root -> " << server_configs->root << std::endl;
//     std::cout << "-- Indexes -> ";
//     for (int i = 0; i < sz(server_configs->indexes); i++)
//         std::cout << server_configs->indexes[i] << " " ;
//     std::cout << std::endl << "-- Allowed methods -> " ;
//     for (int i = 0; i < sz(server_configs->allowed_methods); i++)
//         std::cout << server_configs->allowed_methods[i] << " " ;
//     std::cout << std::endl << "directory_listing -> " << (server_configs->directory_listing ? "on" : "off") << std::endl;
//     std::cout << "auto_indexing -> " << (server_configs->auto_indexing ? "on" : "off") << std::endl;
//     std::cout << "connection -> " << (server_configs->connection ? "on" : "off") << std::endl;
// }

// void print_request(t_request *r)
// {
//     std::cout << "Connection: " << r->connection << std::endl;
//     std::cout << "Content-length: " << r->content_length << std::endl;
//     std::cout << "Http version: " << r->http_version << std::endl;
//     std::cout << "Method: " << r->method << std::endl;
//     std::cout << "Path: " << r->path << std::endl;
// }

// SOCKET Server::get_listener_socket(t_socket *listener)
// {
//     int sockfd, listen_status, bind_status;
//     SA_IN *data = &listener->data;

//     sockfd = socket(AF_INET, SOCK_STREAM, 0);
//     if (sockfd < 0)
//         throw SocketException();
//     fcntl(sockfd, F_SETFL, O_NONBLOCK);
//     bzero(data, sizeof(*data));
//     data->sin_addr.s_addr = INADDR_ANY;
//     data->sin_port = htons(server_configs->port);
//     data->sin_family = AF_INET;
//     listener->data_len = sizeof(listener->data);
//     bind_status = bind(sockfd, (SA *)(data), sizeof(*data));
//     if (bind_status < 0)
//         throw BindException();
//     listen_status = listen(sockfd, 1024);
//     if (listen_status < 0)
//         throw ListenException();
//     return (sockfd);
// }

// void    Server::add_connection(t_network *manager, SOCKET sockfd) // client is a listener socket
// {
//     t_socket    *client;
//     t_socket    *listener;
//     int         index;
//     SOCKET      connection;

//     if (!sz(manager->clients_indexes))
//     {
//         std::cout << "No place left to add this connection!" << std::endl;
//         return ;
//     }
//     client = new t_socket();
//     client->http = new t_http();
//     index = manager->clients_indexes.front();
//     manager->clients_indexes.pop();
//     listener = (t_socket *) manager->changeList[0].udata; // this t_socket surely will be a listener since add_connection has been called
//     connection = accept(sockfd, (SA *)&listener->data, &listener->data_len); // fd of the client
//     if (connection < 0)
//         throw SocketException();
//     fcntl(connection, F_SETFL, O_NONBLOCK);
//     manager->socket_index.insert(std::make_pair(connection, index));
//     client->fd = connection;
//     client->type = CLIENT_READ;
//     client->http->fd = client->fd;
//     client->http->state = WAITING;
//     manager->waiting_connections.push(index);
//     std::cout << "client fd = " << client->fd << std::endl;
//     EV_SET(&manager->changeList[index], connection, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, client);
//     client->http->request_time = time(NULL);
//     manager->connected_clients.insert(std::make_pair(connection, manager->changeList + index)); // new connected client in the table
//     manager->num_connections++;
//     std::cout << "fd = " << manager->changeList[0].ident << " has been added! at index " << index << std::endl;
// }

// void    Server::parse_client_request(t_socket *client)
// {
//     for (int i = 0; i < )
// }

// void    Server::read_request(t_socket *client)
// {
//     SOCKET  fd = client->fd;
//     char    buff[BUFFER_SIZE];
//     char    *line = get_next_line(fd);
//     int     len;
//     std::string s;
//     bool is_finished = client->is_header_complete;

//     if (!line)
//         return ;
//     s = std::string(line);
//     while (1)
//     {
//         line = get_next_line(fd);
//         if (!line)
//             break ;
//         s = std::string(line);
//         client->request->request_lines.push_back(s);
//         client->request->request_len += sz(s);
//         if (line)
//             free(line);
//         is_finished = (s == "\r\n");
//         if (is_finished)
//             break ;
//     }
//     client->request->is_header_complete = is_finished;
//     if (is_finished)
//     {
//         client->request = request_response->parse_client_request(client->request);
//         client->request->request_len += len;
//     }
// }

// do not forget to add a destructor to the t_socket structure
// void    Server::disconnect_client(t_network *manager, int index) // index is the index in event struct
// {
//     t_socket    *client;
//     t_http      *http;
//     int         change_list_index;
//     SOCKET      fd;

//     client = (t_socket *) manager->eventList[index].udata;
//     http = (t_http *) client->http;
//     fd = manager->eventList[index].ident;
//     change_list_index = manager->socket_index[client->fd];
//     if (client->type == LISTENER)
//         std::cout << "I AM A LISTENER I DIIE!" << std::endl;
//     std::cout << fd << " disconnected!" << std::endl;
//     close(fd);
//     NoEvent(&manager->changeList[change_list_index],UNDEFINED, NULL);
//     NoEvent(&manager->eventList[index],UNDEFINED, NULL);
//     manager->changeList[change_list_index].ident = UNDEFINED;
//     kevent(manager->kq, manager->changeList, CONNECTIONS_POLL, NULL, 0, NULL);
//     if (manager->socket_index.find(fd) == manager->socket_index.end())
//         std::cout << "fd is not exist" << std::endl;
//     manager->socket_index.erase(fd);
//     manager->clients_indexes.push(change_list_index);
//     manager->connected_clients.erase(client->fd); // do not delete the appropriate value we still need the k_event struct
//     delete client;
//     delete http;
//     std::cout << "fd = " << fd << " was erased! " << change_list_index << " this index!" << std::endl;
//     manager->num_connections--;
//     std::cout << "Client disconnected!" << std::endl;
// }

// void    Server::add_write_socket(t_network *manager, t_socket *client)
// {

// }

// void    Server::serve_client(t_network *manager, int index) // add it in RequestResponse class instead of here
// {
//     // t_socket    *client;
//     // k_event     *event;
//     // t_request   *req;
//     // int         client_index;

//     // event = manager->eventList + index;
//     // client = (t_socket *) manager->eventList[index].udata;
//     // req = client->request;
//     // if (client->type == CLIENT_READ)
//     // {
//     //     if (client->state == WAITING) // first time
//     //         client->state  = READING_HEADER;
//     //     if (client->state == READING_HEADER)
//     //         request_response->parse_client_request(client);
//     //     else if (client->state == READING_BODY)
//     //         request_response->handle_response(client);
//     // }
//     // if (event->filter == EVFILT_READ) // READ FILTER (1 case -> socket has header to read)
//     // {
//     //     if (inHashMap(manager->read_write, client_index))
//     //         add_write_socket(manager, client);
//     //     if (!client->request->is_header_complete)
//     //         request_response->parse_client_request(client);
//     // }
//     // else // WRITE FILTER
//     //     request_response->handle_response(client);
// }

// void    Server::handle_socket(t_network *manager, int index)
// {
//     t_socket        *sock;
//     t_http          *client;
//     SOCKET_STATE    sock_state;
//     std::string     connection_type;
//     int             index_in_changes;

//     if (manager->eventList[index].flags & EV_EOF) // the socket has been closed and should be removed from the list
//         disconnect_client(manager, index);
//     // else if (manager->eventList[index].fflags == EV_EOF)
//     //     disconnect_client(manager, index);
//     else // read_event which mean we have to read the request
//     {
//         sock = (t_socket *) manager->eventList[index].udata;
//         index_in_changes = manager->socket_index[sock->fd];
//         client = sock->http;
//         client->state = (client->state == WAITING) ? READING_HEADER : client->state;
//         // if (sock->type == CLIENT_WRITE)
//         //     std::cout << "Socket is set for writing" << std::endl;
//         request_response->serve_client(client);
//         // client->state = SERVED;
//         if (client->request)
//             connection_type = client->request->connection;
//         if (!isReadingState(client->state)) // THIS WAS CREATING THE PROBLEM
//             sock->type = CLIENT_WRITE;
//         else
//             sock->type = CLIENT_READ;
//         if (client->state == SERVED) // later on we can add keep-alive connection
//             disconnect_client(manager, index);
//         else
//         {
//             if (sock->type == CLIENT_WRITE) // Well I'd say never check for EVFILT_WRITE
//                 WritingEvent(&manager->changeList[index_in_changes], sock->fd, sock);
//             else if (sock->type == CLIENT_READ)
//                 ReadingEvent(&manager->changeList[index_in_changes], sock->fd, sock);
//         }
//     }
// }

// void    Server::serve_connected_clients(t_network *manager)
// {
//     HashMap<SOCKET, k_event *>::iterator it = manager->connected_clients.begin();
//     HashMap<SOCKET, k_event *>::iterator end = manager->connected_clients.end();
//     SOCKET      sockfd;
//     k_event     *client_kevent;
//     t_socket    *sock;
//     t_http      *client;

//     while (it != end)
//     {
//         sockfd = it->first;
//         client_kevent = it->second;
//         sock = (t_socket *) client_kevent->udata;
//         if (sock->http->state == WAITING && time(NULL) - sock->http->request_time > 30)
//             disconnect_client(manager, manager->socket_index[sock->fd]) ;
//         else
//         {
//             request_response->serve_client(sock->http);
//             if (!isReadingState(sock->http->state))
//             {
//                 sock->type = CLIENT_WRITE;
//                 WritingEvent(client_kevent, sockfd, sock);
//             }
//             else
//             {
//                 sock->type = CLIENT_READ;
//                 ReadingEvent(client_kevent, sockfd, sock);
//             }
//         }
//         std::cout << "After Serving request!" << std::endl;
//         it++;       
//     }
// }

// void    Server::serve_client(t_network *manager, int index)
// {
//     k_event *event = (k_event *) &manager->eventList[index];
//     t_socket *sock = (t_socket *) event->udata;
//     t_http *http = (t_http *) sock->http;
//     int         index_in_changes;

//     index_in_changes = manager->socket_index[sock->fd];
//     std::cout << "Got here and now serving client" << std::endl;
//     if ((http->state == WAITING && time(NULL) - http->request_time > 30) || (http->state == SERVED))
//     {
//         if (http->state == WAITING)
//             std::cout << "Wating" << std::endl;
//         else
//             std::cout << "Served" << std::endl;
//         std::cout << "Time passed: " << time(NULL) - http->request_time << std::endl;
//         std::cout << "CLIENT DISCONNECTED IN [ TIMEOUT BLOCK! ]" << std::endl;
//         disconnect_client(manager, index);
//         return ;
//     }
//     else
//         http->state = (http->state == WAITING) ? READING_HEADER : http->state;
//     request_response->serve_client(http);
//     if (http->state == SERVED)
//     {
//         std::cout << "CLIENT DISCONNECTED IN [ SERVED BLOCK! ]" << std::endl;
//         disconnect_client(manager, index);
//         return ;
//     }
//     if (isReadingState(http->state))
//     {
//         sock->type = CLIENT_READ;
//         ReadingEvent(&manager->changeList[index_in_changes], sock->fd, sock);
//         ReadingEvent(&manager->eventList[index], sock->fd, sock);
//     }
//     else
//     {
//         sock->type = CLIENT_WRITE;
//         WritingEvent(&manager->changeList[index_in_changes], sock->fd, sock);
//         WritingEvent(&manager->eventList[index], sock->fd, sock);
//     }
// }

// void    Server::drop_timeout_connections(t_network *manager) // drops one connection at a time
// {
//     if (!sz(manager->waiting_connections))
//         return ;
//     int index = manager->waiting_connections.front();
//     k_event *event = (k_event *)(manager->changeList + index);
//     t_socket *sock = (t_socket *) event->udata;
//     t_http *http = (t_http *) sock->http;
//     if (http->state != WAITING)
//         manager->waiting_connections.pop();
//     else if ((http->state == WAITING && time(NULL) - http->request_time >= WAITING_TIMEOUT))
//     {
//         std::cout << "Connection has been dropped!" << std::endl;
//         close(sock->fd);
//         NoEvent(&manager->changeList[index],SOCKET_NOT_DEFINED, NULL);
//         manager->socket_index.erase(event->ident);
//         manager->clients_indexes.push(index);
//         manager->connected_clients.erase(event->ident); // do not delete the appropriate value we still need the k_event struct
//         std::cout << "fd = " << sock->fd << " was erased because of timeout! " << index << " this index!" << std::endl;
//         manager->num_connections--;
//         manager->waiting_connections.pop();
//         delete sock;
//         delete http;
//     }
// }

// void    Server::build_with_kqueue()
// {
//     t_network   *manager; // Yes boss!!
//     t_socket    *sock;
//     SOCKET      sockfd;
//     size_t      connections;

//     connections = CONNECTIONS_POLL;
//     sock = new t_socket();
//     manager = new t_network();
//     manager->kq = kqueue();
//     manager->changeList = new k_event[CONNECTIONS_POLL + 1]();
//     manager->eventList = new k_event[CONNECTIONS_POLL + 1]();
//     sock->fd = get_listener_socket(sock);
//     sockfd = sock->fd;
//     sock->type = LISTENER;
//     manager->socket_index.insert(std::make_pair(sock->fd, 0));
//     ReadingEvent(&manager->changeList[0], sock->fd, sock);
//     ReadingEvent(&manager->eventList[0], sock->fd, sock);
//     for (int i = 1; i < CONNECTIONS_POLL + 1; i++)
//     {
//         NoEvent(&manager->changeList[i], SOCKET_NOT_DEFINED, NULL);
//         NoEvent(&manager->eventList[i], SOCKET_NOT_DEFINED, NULL);
//         manager->clients_indexes.push(i);
//     }
//     std::cout << sock->fd << " is socket listener!" << std::endl;
//     if (kevent(manager->kq, manager->changeList, connections, NULL, 0, NULL) != 0)
//         throw MultiplexerException();
//     while (true)
//     {
//         int ret = kevent(manager->kq, manager->changeList, connections, manager->eventList, connections, NULL);
//         for (int i = 0; i < ret; i++)
//         {
//             sock = (t_socket *) manager->eventList[i].udata;
//             std::cout << "BEFORE ......." << manager->eventList[i].ident << std::endl;
//             if (sock->type == LISTENER)
//                 add_connection(manager, sockfd);
//             else
//                 serve_client(manager, i);
//             // drop_timeout_connections(manager); // we cannot handle these connections properly because we need threads and processes api (pthread_create and fork)
//         }
//     }
//     std::cout << "Got here and the program end" << std::endl;
//     close(manager->kq);
// }

// void    Server::run() // the runner where the multiplexing will be (kevent, select, poll)
// {
//     signal(SIGPIPE, SIG_IGN);
//     request_response = new RequestResponse(http_configs, server_configs, dir_configs);
//     std::cout << "GOT HERE" << std::endl;
//     for (auto x: (*dir_configs))
//     {
//         t_location_configs *c = x.second;
//         std::cout << "THESE ARE THE ALLOWED METHODS" << std::endl;
//         for (int i = 0; i < sz(c->allowed_methods); i++)
//             std::cout << c->allowed_methods[i] << std::endl;
//     }
//     if (mult == KQUEUE)
//         build_with_kqueue();
//     // int sockfd, listen_status, bind_status;

//     // bzero(&data, sizeof(data));
//     // data.sin_family = AF_INET;
//     // data.sin_port = htons(this->server_configs->port);
//     // data.sin_addr.s_addr = INADDR_ANY;
//     // sockfd = socket(AF_INET, SOCK_STREAM, 0);
//     // if (sockfd < 0)
//     //     throw SocketException();
//     // bind_status = bind(sockfd, (struct sockaddr *) &data, sizeof(data));
//     // if (bind_status < 0)
//     //     throw BindException();
//     // listen_status = listen(sockfd, this->server_configs->max_connections);
//     // if (listen_status < 0)
//     //     throw ListenException();
//     // while (1)
//     // {
//     //     unsigned int data_size = sizeof(data);
//     //     int acc_conn = accept(sockfd, (struct sockaddr *) &data, &data_size);
//     //     t_request *req = parser->parse_request(acc_conn);
//     //     print_request(req);
//     //     if (req->method == "GET")
//     //         handle_get_request(acc_conn, req);
//     //     else if (req->method == "POST")
//     //         handle_post_request(acc_conn, req);
//     //     else if (req->method == "DELETE")
//     //         handle_delete_request(acc_conn, req);
//     //     delete req;
//     // }
// }