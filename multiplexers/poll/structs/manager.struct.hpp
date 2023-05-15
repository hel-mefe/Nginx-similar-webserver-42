#ifndef MANAGER_STRUCT_HPP
# define MANAGER_STRUCT_HPP

# include "../socket.hpp"
# include "../enums.hpp"

typedef struct manager
{
    struct pollfd *fds;
    std::map<std::string, MethodHandler *>  handlers;
    std::map<SOCKET, t_client *>            clients_map; // maps every SOCKET to its t_socket data
    std::map<SOCKET, t_server *>            servers_map; // maps every SOCKET to its t_server data
    std::deque<int>                         free_slots;

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

    bool    add_client(SOCKET fd, t_server *server)
    {
        if (IN_MAP(clients_map, fd))
            return false ;
        int slot = get_slot();
        if (slot == -1)
            std::cout << "[SLOTS PROBLEM]: no slot exist" << std::endl;
        t_client *client = new t_client(fd, slot, server);
        clients_map.insert(std::make_pair(fd, client));
        fds[slot].fd = fd;
        fds[slot].events = POLLIN | POLLOUT | POLLHUP;
        client->request_time = time(NULL);
        std::cout << "Client has been added succesfully!" << std::endl;
        return true ;
    }

    bool    remove_client(SOCKET fd)
    {
        if (!IN_MAP(clients_map, fd))
            return false ;
        close(fd);
        t_client *data = clients_map[fd];
        int i = data->slot;
        clients_map.erase(fd);
        fds[i].fd = -1;
        fds[i].events = 0;
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

#endif
