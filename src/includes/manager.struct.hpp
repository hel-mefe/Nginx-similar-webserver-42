#pragma once

# include "socket.hpp"
# include "enums.hpp"
# include "interface.methodhandler.hpp"

typedef struct manager
{
    struct pollfd *fds;
    std::map<std::string, MethodHandler *>  handlers;
    std::map<SOCKET, t_client *>            clients_map; // maps every SOCKET to its t_socket data
    std::map<SOCKET, t_server *>            servers_map; // maps every SOCKET to its t_server data
    std::deque<int>                         free_slots;
    std::string                             cwd;
    int                                     client_num;

    bool is_listener(SOCKET fd)
    {
        return IN_MAP(servers_map, fd);
    }

    // bool in_map(SOCKET fd)
    // {
    //     return clients_map.find(fd) != clients_map.end();
    // }

    struct pollfd *clone_fds()
    {
        struct pollfd *_fds = new struct pollfd[MAX_FDS];
        for (int i = 0; i < MAX_FDS; i++)
        {
            _fds[i].fd = fds[i].fd;
            _fds[i].events = fds[i].events;
            _fds[i].revents = fds[i].revents;
        }
        return _fds;
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

    int     get_free_slot()
    {
        int free_slot = UNDEFINED;
        
        for (int i = 0; i < MAX_FDS; i++)
        {
            if (fds[i].fd == UNDEFINED)
            {
                free_slot = i ;
                break ;
            }
        }
        return (free_slot) ;
    }

    bool    add_client(SOCKET fd, t_server *server)
    {
        if (IN_MAP(clients_map, fd))
            return false ;
        int slot = get_free_slot();
        if (slot == -1)
            std::cerr << "[SLOTS PROBLEM]: no slot exist" << std::endl;
        t_client *client = new t_client(fd, slot, server);
        client->cwd = cwd;
        clients_map.insert(std::make_pair(fd, client));
        fds[slot].fd = fd;
        fds[slot].events = POLLIN | POLLOUT | POLLHUP;
        client->request_time = time(NULL);
        return true ;
    }

    void    move_clients_left(int start)
    {
        int i = start + 1;

        for (; i < MAX_FDS && fds[i].fd != UNDEFINED; i++)
            fds[i - 1].fd = fds[i].fd;
    }

    bool    remove_client(SOCKET fd)
    {
        if (!IN_MAP(clients_map, fd))
            return false ;
        if (close(fd))
            std::cout << RED_BOLD << "[ FD WAS NOT CLOSED ]" << std::endl;
        t_client *data = clients_map[fd];
        int i = data->slot;
        fds[i].fd = -1;
        fds[i].events = 0;
        fds[i].revents = 0;
        move_clients_left(i);
        clients_map.erase(fd);
        add_slot(i);
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

} t_manager;

