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
# define interval(a, b, x) (x > a && x < b)
# define IS_CACHETM_VALID(time_created, time_expired, time_period) (interval(time_created, time_expired, time(NULL)) && time_expired - time_created == time_period)
# define IS_CACHE_VALID(c) ((!access(c->rq_file.c_str(), R_OK) && !access(c->s_file.c_str(), R_OK) && c->t_created >= c->t_rq_last_modified && c->t_created >= c->t_s_last_modified))
# define IS_CACHE_EXPIRED(t_expired) ((time(NULL) >= t_expired))
# define SET_CACHE_OFF(http_configs) (http_configs->proxy_cache = false, http_configs->proxy_cache_register = false, http_configs->proxy_cache_max_size = UNDEFINED, http_configs->proxy_cache_max_time = UNDEFINED)
# define IS_CACHE_PASSED_SIZE(cache_size) ((cache_size >= get_cache_folder_size("caches/")))
# define IS_NEW_CACHE_PROVIDED(http_configs) ((http_configs->proxy_cache_max_size != UNDEFINED || http_configs->proxy_cache_max_time != UNDEFINED))

class Webserver
{
    private:
        std::vector<t_server *>                     *servers;
        t_http_configs                              *http_configs;
        ConfigFileParser                            *parser;
        MultiplexerInterface                        *multiplexer;
        HashMap<int, std::string>                   *codes;
        HashMap<std::string, std::string>           *mimes;
        HashMap<std::string, t_cache *>             *caches;
        t_cli                                       *cli;
        std::string                                 config_file;
        std::queue<std::pair<MSG_TYPE, std::string> >    msgs_queue;
        bool                                        is_warning_set;

    public:
        Webserver();
        Webserver(std::string);
        Webserver(std::string, MultiplexerInterface *);
        Webserver(const Webserver& w);
        Webserver& operator=(const Webserver& w);
        ~Webserver();

        bool    is_parsed_data_valid();
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
        void                        parse_cacherc();
        bool                        parse_cache_line(std::string &line);
        bool                        parse_cachetm(t_http_configs *http_configs);
        void                        set_cache_data(t_cache *c, std::string &line);
        void                        reset_cache();
        void                        write_cache_to_cachetm(t_http_configs *http_configs);

        void                        set_all_warnings();
        void                        set_location_defined_multiple_times_warning(t_server *s);
        void                        set_cgi_bins_warning(t_server *s);
        void                        set_cgi_bin_warning(std::string server_name, std::map<std::string, std::string> *cgi_path);
        void                        set_redirection_loop_warnings(t_server *s);
        bool                        is_there_a_cycle(std::map<std::string, t_location_configs*> *dir_configs, std::string l_name);

} ;