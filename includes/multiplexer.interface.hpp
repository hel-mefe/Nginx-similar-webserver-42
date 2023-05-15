#ifndef MULTIPLEXER_HPP
# define MULTIPLEXER_HPP

# include <iostream>
# include "structures.hpp"

class MultiplexerInterface
{
    public:
        virtual void    multiplex() = 0;
        virtual void    set_configs(t_http_configs *) = 0;
        virtual void    set_servers(std::vector<t_server *> *) = 0;
        virtual void    set_mimes(HashMap<std::string, std::string> *) = 0;
        virtual void    set_codes(HashMap<int, std::string> *) = 0;
        virtual         ~MultiplexerInterface(){};
} ;

#endif