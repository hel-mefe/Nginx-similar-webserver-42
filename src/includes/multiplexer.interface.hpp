#pragma once

# include <iostream>
# include "structs.hpp"

#define IS_HTTP_STATE(state) (state == READING_HEADER)
#define IS_METHOD_STATE(state) (state == SERVING_GET || state == SERVING_POST || state == SERVING_DELETE || state == SERVING_OPTIONS)

class MultiplexerInterface
{
    public:
        virtual void    multiplex() = 0;
        virtual void    set_configs(t_http_configs *) = 0;
        virtual void    set_servers(std::vector<t_server *> *) = 0;
        virtual void    set_mimes(HashMap<std::string, std::string> *) = 0;
        virtual void    set_codes(HashMap<int, std::string> *) = 0;
        virtual void    set_caches(HashMap<std::string, t_cache *> *) = 0;
        virtual         ~MultiplexerInterface(){};
} ;