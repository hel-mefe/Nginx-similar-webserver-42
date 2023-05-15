#ifndef HTTP_PARSER_HPP
# define HTTP_PARSER_HPP

# include "socket.hpp"
# include "../../includes/structures.hpp"
# include "http_handler.utils.hpp"

typedef struct request t_request;

class HttpParser
{
    public:
        HttpParser(){};
        ~HttpParser(){};     

        bool    read_header(t_client *client);
        void    parse_request(t_client *client);
        void    parse_first_line(t_request *req);
} ;


#endif