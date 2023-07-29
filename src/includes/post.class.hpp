#ifndef POST_HPP
# define POST_HPP

# include "interface.methodhandler.hpp"
# include "http_handler.class.hpp"
# include "socket.hpp"
# include <fstream>
#include <list>

class Post : public MethodHandler
{
    public:
        Post();
        void    serve_client(t_client*);
        ~Post();
} ;

#endif