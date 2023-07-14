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
        void    handle_static_file(t_client *client);
        void    handle_directory_listing(t_client *client);
        void    list_directories(t_client *client);
        ~Get(){}
} ;

#endif