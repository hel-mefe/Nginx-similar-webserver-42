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
        char**  convert_cgi_env(t_client*);
        void    fill_cgi_env(t_client*);
        void    client_served(t_client*);
        void    serve_client(t_client*);
        void    parse_chunked_body(t_client*);
        void    parse_non_chunked_body(t_client*);
        void    serve_cgi(t_client*, char**, int);
        void    create_file(t_client*);
        void    fill_response(t_client*, int, std::string, bool);
        void    parse_cgi_output(t_client* client);
        ~Post();
} ;

#endif