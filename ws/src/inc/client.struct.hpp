#ifndef CLIENT_STRUCT_HPP
# define CLIENT_STRUCT_HPP

# include "request.struct.hpp"
# include "response.struct.hpp"

typedef struct client
{
    SOCKET          fd;
    int             slot;
    time_t          request_time;
    CLIENT_STATE    state;
    t_request       *request;
    t_response      *response;
    t_server        *server;

    client(SOCKET _fd)
    {
        fd = _fd;
        request_time = time(NULL);
        state = WAITING;
        server = nullptr;
        request = new t_request();
        response = new t_response();
    }

    client(SOCKET _fd, int _slot)
    {
        fd = _fd;
        slot = _slot;
        request_time = time(NULL);
        state = WAITING;
        server = nullptr;
        request = new t_request();
        response = new t_response();
    }

    client(SOCKET _fd, int _slot, t_server *_server)
    {
        fd = _fd;
        slot = _slot;
        request_time = time(NULL);
        state = WAITING;
        server = _server;
        request = new t_request();
        response = new t_response();
    }


    ~client()
    {
        if (request)
            delete request ;
        if (response)
            delete response ;
    }

    void    reset(CLIENT_STATE _state)
    {
        state = _state;
        if (response)
            delete response;
        if (request)
            delete request;
        std::cout << YELLOW_BOLD << "Connection has been reset -> " << (_state == KEEP_ALIVE ? "KEEP_ALIVE" : "CLOSED") << WHITE << std::endl;
        response = new t_response();
        request = new t_request();
    }

    void    set_request_time()
    {
        request_time = time(NULL);
    }
} t_client;

#endif