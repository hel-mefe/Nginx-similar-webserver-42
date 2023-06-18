#pragma once

# include "socket.hpp"
# include "structs.hpp"
# include "http_handler.utils.hpp"
# include "client.struct.hpp"

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