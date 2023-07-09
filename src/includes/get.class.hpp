#ifndef GET_HPP
# define GET_HPP

# include "interface.methodhandler.hpp"
# include "socket.hpp"
# include <dirent.h>

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
        void    handle_directory_listing(t_client *client);
        void    list_directories(t_client *client);

        char    **convert_env_map(std::map<std::string, std::string> &m);
        void    serve_directory_listing(t_client *client);
        void    write_schunk(SOCKET fd, std::string &s, int len);
        
} ;

#endif