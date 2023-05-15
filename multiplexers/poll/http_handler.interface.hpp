#ifndef HTTP_HANDLER_INTERFACE_HPP
# define HTTP_HANDLER_INTERFACE_HPP

# include "socket.hpp"
# include "http_parser.class.hpp"


class HttpHandlerInterface
{
    public:
        virtual void    handle_http(t_client *) = 0;
        // // virtual void    parse_request(t_client *) = 0;
        // // virtual void    architect_response(t_client *) = 0;
        virtual void    set_codes(std::map<int, std::string> *) = 0;
        virtual void    set_mimes(std::map<std::string, std::string> *) = 0;
        virtual ~HttpHandlerInterface(){};
} ;

#endif