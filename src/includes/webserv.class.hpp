#pragma once

# include "header.hpp"
# include "structs.hpp"
# include "parser.class.hpp"
# include "multiplexer.interface.hpp"
# include "poll.class.hpp"
#ifdef __FREEBSD__
 # include "kqueue.class.hpp"
#endif

#ifndef __FREBSD
 #ifdef __APPLE__
  # include "kqueue.class.hpp"
 #endif
#endif

#ifdef __linux__
 # include "epoll.class.hpp"
#endif

# define IS_CACHE_VALID(c) ((!access(c->rq_file.c_str(), R_OK) && !access(c->s_file.c_str(), R_OK) && c->t_created >= c->t_rq_last_modified && c->t_created >= c->t_s_last_modified))
# define IS_CACHE_EXPIRED(t_expired) ((time(NULL) > t_expired))
# define SET_CACHE_OFF(http_configs) (http_configs->proxy_cache = false, http_configs->proxy_cache_register = false, http_configs->proxy_cache_max_size = UNDEFINED, http_configs->proxy_cache_max_time = UNDEFINED)
# define IS_CACHE_PASSED_SIZE(cache_size) ((cache_size >= get_cache_folder_size("caches/")))

class Webserver
{
    private:
        std::vector<t_server *>             *servers;
        t_http_configs                      *http_configs;
        ConfigFileParser                    *parser;
        MultiplexerInterface                *multiplexer;
        HashMap<int, std::string>           *codes;
        HashMap<std::string, std::string>   *mimes;
        HashMap<std::string, t_cache *>     *caches;
        t_cli                               *cli;
        std::string                         config_file;
    public:
        Webserver();
        Webserver(std::string);
        Webserver(std::string, MultiplexerInterface *);
        Webserver(const Webserver& w);
        Webserver& operator=(const Webserver& w);
        ~Webserver();

        bool    parse_config_file();
        void    run();

        void    init_mimes();
        void    init_codes();
        void    set_multiplexer();
        void    set_cli(t_cli *);
    
        // Debugging lines to check the data that was parsed
        void    print_global_http_data();
        void    print_servers_data();
        void    print_all_data();

        //Warnings
        std::vector<std::string>    generate_all_warnings();
        void                        set_cgi_warning(t_server *server, std::vector<std::string> &wrs);
        void                        set_redefined_location_warning(t_server *server, std::vector<std::string> &wrs);
        void                        set_invalid_root_warning(t_server *server, std::vector<std::string> &wrs);
        void                        set_redirection_loop_warning(t_server *server, std::vector<std::string> &wrs);
        void                        create_redirection_graph(t_server *server, HashMap<std::string, std::string> &graph);
        bool                        is_cycled(HashMap<std::string, std::string> &graph);
        bool                        is_root_cycled(HashMap<std::string, std::string> &graph, std::string start);

        //Cache
        void                        parse_cache(std::vector<std::string> &);
        bool                        parse_cache_line(std::string &line);
        bool                        set_cache_time_data(t_http_configs *http_configs, std::vector<std::string> &warnings);
        void                        set_cache_data(t_cache *c, std::string &line);

} ;