#ifndef SOCKET_STRUCT_HPP
# define SOCKET_STRUCT_HPP

# include "../socket.hpp"
# include "client.struct.hpp"
# include "../enums.hpp"

// default is a socket listener
typedef struct socket
{
    SOCKET      fd;
    PORT        port;
    bool        is_listener;
    int         slot;
    t_client    *data;
    socket()
    {
        is_listener = true;
        port = UNDEFINED_PORT;
        data = new t_client(fd);
    }

    socket(SOCKET _fd, PORT _port, bool _is_listener, int _slot)
    {
        fd = _fd;
        port = _port;
        is_listener = _is_listener;
        slot = _slot;
        if (!is_listener)
            data = new t_client(fd);
    }
} t_socket ;

#endif