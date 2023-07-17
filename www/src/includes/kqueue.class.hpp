#pragma once

# include "header.hpp"
# include "structs.hpp"
# include "exceptions.hpp"
# include "multiplexer.interface.hpp"
# include "globals.hpp"
# include "socket.hpp"
# include "http_handler.class.hpp"
# include "post.class.hpp"
# include "delete.class.hpp"
# include "get.class.hpp"
# include "options.class.hpp"
# include "kqueueManager.struct.hpp"


class Kqueue : public MultiplexerInterface
{
    private:
        t_http_configs                          *configs;
        t_kqueue_manager                        *manager;
        std::vector<t_server *>                 *servers;
        HashMap<int, std::string>               *codes;
        HashMap<std::string, std::string>       *mimes;
        HttpHandlerInterface                    *http_handler;
        HashMap<std::string, MethodHandler*>    *method_handlers;
    
    public:
        Kqueue() : configs(nullptr), servers(nullptr)
        {
            method_handlers = new HashMap<std::string, MethodHandler*>();
            method_handlers->insert(std::make_pair("GET", new Get()));
            method_handlers->insert(std::make_pair("POST", new Post()));
            method_handlers->insert(std::make_pair("DELETE", new Delete()));
            method_handlers->insert(std::make_pair("OPTIONS", new Options()));
        }
        Kqueue(t_http_configs *_configs, std::vector<t_server *>* _servers)
        {
            manager = nullptr;
            configs = _configs;
            servers = _servers;
        }
        Kqueue(t_http_configs *_configs, std::vector<t_server *>* _servers, HashMap<int, std::string> *_codes, HashMap<std::string, std::string> *_mimes)
        {
            manager = nullptr;
            configs = _configs;
            servers = _servers;
            codes = _codes;
            mimes = _mimes;
        }
        ~Kqueue(){}

        // Interface methods
        void    multiplex();
        void    set_configs(t_http_configs *_configs){configs = _configs;}
        void    set_servers(std::vector<t_server *> *_servers){servers = _servers;}
        void    set_codes(HashMap<int, std::string> *_codes){codes = _codes;}
        void    set_mimes(HashMap<std::string, std::string> *_mimes){mimes = _mimes;} 

        void    set_manager();

        void        handle_connection(t_kqueue_manager *manager, SOCKET fd);
        void        handle_disconnection(t_kqueue_manager *manager, SOCKET fd);
        void        handle_client(t_kqueue_manager *manager, SOCKET fd);

        void                write_error(const std::string err_msg);
        struct sockaddr_in* getsocketdata(PORT port);
        SOCKET              getsocketfd(int port);
} ;
