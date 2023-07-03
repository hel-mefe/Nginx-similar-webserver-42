#pragma once

# include "socket.hpp"
# include "http_parser.class.hpp"
# include "client.struct.hpp"


class HttpHandlerInterface
{
    public:
        virtual void    handle_http(t_client *) = 0;
        virtual void    set_codes(std::map<int, std::string> *) = 0;
        virtual void    set_mimes(std::map<std::string, std::string> *) = 0;
        virtual ~HttpHandlerInterface(){};
} ;
