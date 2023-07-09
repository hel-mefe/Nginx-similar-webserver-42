#pragma once

# include "header.hpp"
# include "structs.hpp"
# include "parser.class.hpp"
# include "multiplexer.interface.hpp"
# include "poll.class.hpp"

class Webserver
{
    private:
        std::vector<t_server *>             *servers;
        t_http_configs                      *http_configs;
        ConfigFileParser                    *parser;
        MultiplexerInterface                *multiplexer;
        HashMap<int, std::string>           *codes;
        HashMap<std::string, std::string>   *mimes;
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

        // Debugging lines to check the data that was parsed
        void    print_global_http_data();
        void    print_servers_data();
        void    print_all_data();
} ;