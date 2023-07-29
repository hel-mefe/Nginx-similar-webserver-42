#pragma once

# include "socket.hpp"
# include "enums.hpp"
# include "interface.methodhandler.hpp"

#define MAX_EPOLL_FDS 100000 //C10k problem
#define READY_READ EVFILT_READ
#define READY_WRITE EVFILT_WRITE

typedef struct epoll_manager
{
    struct epoll_event                      *rEvents;
    int                                     epoll_fd;
    std::map<std::string, MethodHandler *>  handlers;
    std::map<SOCKET, t_client *>            clients_map; // maps every SOCKET to its t_socket data
    std::map<SOCKET, t_server *>            servers_map; // maps every SOCKET to its t_server data
    std::deque<int>                         free_slots;
    std::string                             cwd;
    int                                     client_num;

    epoll_manager()
    {
        rEvents = new struct epoll_event[MAX_EPOLL_FDS];
        epoll_fd = epoll_create(0);
    }

    bool is_listener(SOCKET fd)
    {
        return IN_MAP(servers_map, fd);
    }
    
    /*****
    * Abstract high level functions for Epoll
    * These functions are used to trigger Epoll events
    *****/

    void set_epoll_event(int fd, int events)
    {
        struct epoll_event event;

        event.events = events | EPOLLHUP;
        ev.data.fd = fd;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event) == -1) 
        {
            std::cerr << RED_BOLD << "[Webserv42]: Epoll internal multiplexing issue" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    void reset_epoll_event(int fd)
    {
        struct epoll_event event;

        event.events = 0;
        event.data.fd = fd;
        if (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event) == -1) 
        {
            std::cerr << RED_BOLD << "[Webserv42]: Epoll internal multiplexing issue" << std::endl;
            exit(EXIT_FAILURE);
        }
    }


    int get_slot() // -1 no slots availabel
    {
        if (!sz(free_slots))
            return -1 ;
        int slot = free_slots.front();
        free_slots.pop_front();
        return slot;
    }

    bool add_slot(int slot)
    {
        free_slots.push_back(slot);
        return true ;
    }

    t_client *get_client(SOCKET fd)
    {
        if (!IN_MAP(clients_map, fd))
            return nullptr ;
        return clients_map[fd];
    }

    // int     get_free_slot()
    // {
    //     int free_slot = UNDEFINED;
        
    //     for (int i = 0; i < MAX_FDS; i++)
    //     {
    //         if (fds[i].fd == UNDEFINED)
    //         {
    //             free_slot = i ;
    //             break ;
    //         }
    //     }
    //     return (free_slot) ;
    // }

    void    printClientsMap()
    {
        for (std::map<SOCKET, t_client *>::iterator it = clients_map.begin(); it != clients_map.end(); it++)
            std::cout << it->first << " - " << std::endl;
    }

    bool    add_client(SOCKET fd, t_server *server)
    {
        if (IN_MAP(clients_map, fd))
            return false ;
        t_client *client = new t_client(fd, server);
        client->cwd = cwd;
        clients_map.insert(std::make_pair(fd, client));
        set_epoll_event(fd, EPOLLOUT); // listen for write event for waiting state
        client->request_time = time(NULL);
        return true ;
    }

    bool    remove_client(SOCKET fd)
    {
        if (!IN_MAP(clients_map, fd))
            return false ;
        std::cout << "Working on " << fd << std::endl;
        printClientsMap();
        if (close(fd))
            std::cout << RED_BOLD << "[ FD WAS NOT CLOSED ]" << std::endl;
        t_client *data = clients_map[fd];
        clients_map.erase(fd);
        if ((data && data->request && data->request->method == "POST") || IS_HTTP_STATE(data->state))
        {
            reset_epoll_event(fd);
            set_epoll_event(fd, EPOLLIN);
        }
        else
        {
            reset_epoll_event(fd);
            set_epoll_event(fd, EPOLLOUT);
        }
        delete data;
        return true;
    }

    bool add_server(SOCKET _fd, t_server *_server)
    {
        if (IN_MAP(servers_map, _fd))
            return false ;
        servers_map.insert(std::make_pair(_fd, _server));
        return true ;
    }

    bool delete_server(SOCKET _fd)
    {
        if (!IN_MAP(servers_map, _fd))
            return false ;
        servers_map.erase(_fd);
        return true ;
    }

    t_server    *get_server(SOCKET _fd)
    {
        if (!IN_MAP(servers_map, _fd))
            return nullptr ;
        return servers_map[_fd];
    }

} t_epoll_manager;

