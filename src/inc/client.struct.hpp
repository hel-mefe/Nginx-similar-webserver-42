#ifndef CLIENT_STRUCT_HPP
# define CLIENT_STRUCT_HPP

# include "request.struct.hpp"
# include "response.struct.hpp"

typedef struct client
{
    SOCKET          fd;
    int             slot;
    int             client_num;
    time_t          request_time;
    CLIENT_STATE    state;
    t_request       *request;
    t_response      *response;
    t_server        *server;
    std::string     cwd;

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
        std::cout << YELLOW_BOLD << "--> Client with " << fd << " has been created!" << std::endl;
    }


    ~client()
    {
        if (request)
            delete request ;
        if (response)
            delete response ;
    }

    void    check()
    {
        std::cout << "SLOT => " << slot << std::endl;
        std::cout << "Client num => " << client_num << std::endl;
        if (request)
            std::cout << "Request is not null" << std::endl;
        std::cout << request->method << std::endl;
        std::cout << "not in request" << std::endl;
        if (response)
        {
            std::cout << "response is not null" << std::endl;
            std::cout << response->http_version << std::endl;
        }
        std::cout << "not in response" << std::endl;
        if (server)
        {
            std::cout << "server is not null" << std::endl;
            std::cout << "not in server" << std::endl;
        }
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