#ifndef GET_HPP
# define GET_HPP

# include "interface.methodhandler.hpp"
# include "socket.hpp"

#define SOCKET int

class Get : public MethodHandler
{
    public:
        Get(){}
        void    serve_client(t_client *client);
        ~Get(){}

        void    write_hex(SOCKET, int);
        void    write_chunk(SOCKET, unsigned char *, int);

        void    handle_static_file(t_client *client);
        void    handle_cgi(t_client *client);
        void    serve_cgi(t_client *client);
} ;

#endif