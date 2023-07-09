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
        void    client_served(t_client*);
        void    serve_client(t_client*);
        void    parse_chunked_body(t_client*);
        void    parse_non_chunked_body(t_client*);
        void    create_file(t_client*);
        ~Post();
} ;

#endif