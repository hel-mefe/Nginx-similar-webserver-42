#ifndef OPTIONS_HPP
# define OPTIONS_HPP

# include "interface.methodhandler.hpp"
# include "socket.hpp"
# include <dirent.h>

#define SOCKET int

class Options : public MethodHandler
{
    public:
        Options(){}
        void            serve_client(t_client *client);
        std::string     get_allowed_methods(t_client *client);
        ~Options(){}
} ;

#endif