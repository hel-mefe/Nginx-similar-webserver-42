#ifndef POLL_HPP
# define POLL_HPP

# include "../../includes/header.hpp"
# include "../../includes/structures.hpp"
# include "../../includes/exceptions.hpp"
# include "../../includes/multiplexer.interface.hpp"
# include "../../includes/globals.hpp"
# include "socket.hpp"
# include "http_handler.class.hpp"
# include "methods/get.class.hpp"
# include "methods/post.class.hpp"
# include "methods/delete.class.hpp"


class Poll : public MultiplexerInterface
{
    private:
        t_http_configs                          *configs;
        t_manager                               *manager;
        std::vector<t_server *>                 *servers;
        HashMap<int, std::string>               *codes;
        HashMap<std::string, std::string>       *mimes;
        HttpHandlerInterface                    *http_handler;
        HashMap<std::string, MethodHandler*>    *method_handlers;
    
    public:
        Poll() : configs(nullptr), servers(nullptr)
        {
            method_handlers = new HashMap<std::string, MethodHandler*>();
            method_handlers->insert(std::make_pair("GET", new Get()));
            method_handlers->insert(std::make_pair("POST", new Post()));
            method_handlers->insert(std::make_pair("DELETE", new Delete()));
        }
        Poll(t_http_configs *_configs, std::vector<t_server *>* _servers)
        {
            manager = nullptr;
            configs = _configs;
            servers = _servers;
        }
        Poll(t_http_configs *_configs, std::vector<t_server *>* _servers, HashMap<int, std::string> *_codes, HashMap<std::string, std::string> *_mimes)
        {
            manager = nullptr;
            configs = _configs;
            servers = _servers;
            codes = _codes;
            mimes = _mimes;
        }
        ~Poll(){}

        // Interface methods
        void    multiplex();
        void    set_configs(t_http_configs *_configs){configs = _configs;}
        void    set_servers(std::vector<t_server *> *_servers){servers = _servers;}
        void    set_codes(HashMap<int, std::string> *_codes){codes = _codes;}
        void    set_mimes(HashMap<std::string, std::string> *_mimes){mimes = _mimes;} 

        t_manager *get_manager();

        void        handle_connection(t_manager *manager, SOCKET fd);
        void        handle_disconnection(t_manager *manager, SOCKET fd);
        void        handle_client(t_manager *manager, SOCKET fd);
} ;

#endif