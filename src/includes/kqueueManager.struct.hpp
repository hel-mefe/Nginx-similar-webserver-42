#pragma once

# include "socket.hpp"
# include "enums.hpp"
# include "interface.methodhandler.hpp"

#define MAX_KQUEUE_FDS 100000 //C10k problem
#define READY_READ EVFILT_READ
#define READY_WRITE EVFILT_WRITE

typedef struct kqueueManager
{
    struct kevent                           *rEvents;
    int                                     kq;
    std::map<std::string, MethodHandler *>  handlers;
    std::map<SOCKET, t_client *>            clients_map; // maps every SOCKET to its t_socket data
    std::map<SOCKET, t_server *>            servers_map; // maps every SOCKET to its t_server data
    std::deque<int>                         free_slots;
    std::string                             cwd;
    int                                     client_num;

    kqueueManager()
    {
        rEvents = new struct kevent[MAX_KQUEUE_FDS];
        kq = kqueue();
    }

    bool is_listener(SOCKET fd)
    {
        return IN_MAP(servers_map, fd);
    }
    
    /*****
    * Abstract high level functions for Kqueue
    * These functions are kind of used to trugger kqueue events
    *****/

    void add_kqueue_event(int fd, int filter, void *udata){
        struct kevent event;

        EV_SET(&event, fd, filter, EV_ADD, 0, 0, udata);
        kevent(kq, &event, 1, NULL, 0, NULL);
    }

    void delete_kqueue_event(int fd, int filter)
    {
        struct kevent event;

        EV_SET(&event, fd, filter, EV_DELETE, 0, 0, NULL);
        kevent(kq, &event, 1, NULL, 0, NULL);
    }

    void disable_kqueue_event(int fd, int filter)
    {
        struct kevent event;

        EV_SET(&event, fd, filter, EV_DISABLE, 0, 0, NULL);
        kevent(kq, &event, 1, NULL, 0, NULL);   
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
        add_kqueue_event(fd, EVFILT_WRITE, client); // EVFILT_WRITE cuz WAITING state is not HTTP_STATE and not METHOD_STATE
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
            delete_kqueue_event(fd, EVFILT_READ);
        else
            delete_kqueue_event(fd, EVFILT_WRITE);
        delete data;
        return true ;
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

} t_kqueue_manager;

