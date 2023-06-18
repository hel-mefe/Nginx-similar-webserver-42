# pragma once

# include "header.hpp"
# include "globals.hpp"

# define DEFAULT_MAX_CONNECTIONS 1000
# define DEFAULT_MAX_BODY_SIZE 2048

typedef struct ServerAttributes
{
    std::string                         server_name;
    PORT                                port;
    std::string                         root;
    std::vector<std::string>            indexes;
    std::vector<std::string>            allowed_methods;
    std::vector<std::string>            allowed_cgi;
    std::vector<std::string>            locations;
    std::vector<std::string>            pages_404;
    HashMap<std::string, std::string>   extension_cgi; // <extension, cgi_program_name>
    HashSet<std::string>                allowed_methods_set;
    HashSet<std::string>                allowed_cgi_set;
    HashSet<std::string>                locations_set;
    HashSet<std::string>                indexes_set;
    HashSet<std::string>                pages_404_set;
    int                                 max_connections;
    int                                 max_body_size;
    bool                                directory_listing;
    bool                                auto_indexing;
    bool                                connection; // keep-alive or closed

    ServerAttributes() : max_connections(DEFAULT_MAX_CONNECTIONS), max_body_size(DEFAULT_MAX_BODY_SIZE) {}
}   t_server_configs;

typedef struct LocationConfigs
{
    std::string                         root;
    std::string                         redirection;
    std::vector<std::string>            indexes;
    std::vector<std::string>            allowed_methods;
    std::vector<std::string>            allowed_cgi;
    std::vector<std::string>            locations;
    std::vector<std::string>            pages_404;
    std::vector<std::string>            pages_301;
    HashSet<std::string>                indexes_set;
    HashSet<std::string>                allowed_methods_set;
    HashSet<std::string>                pages_404_set;
    HashSet<std::string>                pages_301_set;
    bool                                auto_indexing;
    bool                                connection;
    bool                                upload;
    bool                                directory_listing;

    LocationConfigs()
    {
        connection = false;
        auto_indexing = false;
        upload = false;
        directory_listing = false;
    }
}   t_location_configs;

typedef struct HttpConfigs
{
    std::string                         root;
    std::vector<std::string>            indexes;
    std::vector<std::string>            allowed_methods;
    std::vector<std::string>            allowed_cgi;
    std::vector<std::string>            locations;
    HashMap<std::string, std::string>   extension_cgi; // <extension, cgi_program_name>
    HashSet<std::string>                allowed_methods_set;
    HashSet<std::string>                allowed_cgi_set;
    HashSet<std::string>                locations_set;
    bool                                directory_listing;
    bool                                auto_indexing;
    bool                                connection; // keep-alive or closed

    HttpConfigs()
    {
        directory_listing = true;
        auto_indexing = false;
        connection = false;
    }
}   t_http_configs;

typedef struct server
{
    t_server_configs                                        *server_configs;
    t_http_configs                                          *http_configs;
    HashMap<std::string, t_location_configs *>              *dir_configs; // directory configs (dir -> its configs)

    server()
    {
        server_configs = new t_server_configs();
        http_configs = new t_http_configs();
        dir_configs = new HashMap<std::string, t_location_configs *>();
    }

    ~server(){}

    void    set_location_map(std::string path, t_location_configs *configs)
    {
        dir_configs->insert(std::make_pair(path, configs));
    }

    void    set_server_configs(t_server_configs *_server_configs)
    {
        if (server_configs)
            delete server_configs;
        std::cout << "YES CALLED" << std::endl;
        server_configs = _server_configs;
    }

    void    print_http()
    {
        std::cout << GREEN_BOLD << "**** PRINTING HTTP DATA *****" << WHITE_BOLD << std::endl;
        std::cout << "root -> " <<  http_configs->root << std::endl;
        std::cout << "indexes -> " ;
        for(int i = 0; i < sz(http_configs->indexes); i++)
            std::cout << http_configs->indexes[i] << " " ;
        std::cout << std::endl;
        std::cout << "allowed methods -> " ;
        for (int i = 0; i < sz(http_configs->allowed_methods); i++)
            std::cout << http_configs->allowed_methods[i] << " " ;
        std::cout << std::endl;
        std::cout << "directory listing -> " << (http_configs->directory_listing ? "on" : "off") << std::endl;
        std::cout << "auto_indexing ->" << (http_configs->auto_indexing ? "on" : "off") << std::endl;
        std::cout << GREEN_BOLD << "**** END PRINTING HTTP DATA *****\n" << WHITE_BOLD << std::endl;
    }

    void    print_server()
    {
        std::cout << RED_BOLD << "**** PRINTING SERVER DATA *****" << WHITE_BOLD << std::endl;
        std::cout << "root -> " <<  server_configs->root << std::endl;
        std::cout << "indexes -> " ;
        for(int i = 0; i < sz(server_configs->indexes); i++)
            std::cout << server_configs->indexes[i] << " " ;
        std::cout << std::endl;
        std::cout << "allowed methods -> " ;
        for (int i = 0; i < sz(server_configs->allowed_methods); i++)
            std::cout << server_configs->allowed_methods[i] << " " ;
        std::cout << std::endl;
        std::cout << "directory listing -> " << (server_configs->directory_listing ? "on" : "off") << std::endl;
        std::cout << "auto_indexing ->" << (server_configs->auto_indexing ? "on" : "off") << std::endl;
        std::cout << "client_max_body_size ->" << server_configs->max_body_size << std::endl;
        std::cout << RED_BOLD << "**** END PRITING SERVER DATA *****\n" << WHITE_BOLD << std::endl;

    }


    void    print_location_data(t_location_configs *conf)
    {
        std::cout << "root -> " << conf->root << std::endl;
        std::cout << "redirection -> " << conf->redirection << std::endl;
        for (int i = 0; i < sz(conf->pages_301); i++)
            std::cout << conf->pages_301[i] << " " ;
        std::cout << "indexes -> " ;
        for (int i = 0; i < sz(conf->indexes); i++)
            std::cout << conf->indexes[i] << " " ;
        std::cout << std::endl;
        std::cout << "allowed_methods -> " ;
        for (int i = 0; i < sz(conf->allowed_methods); i++)
            std::cout << conf->allowed_methods[i] << " " ;
        std::cout << std::endl;
        std::cout << "auto_indexing -> " << (conf->auto_indexing ? "on" : "off") << std::endl;
        std::cout << "connection -> " << (conf->connection ? "on" : "off") << std::endl;
        std::cout << "upload -> " << (conf->upload ? "on" : "off") << std::endl;
        std::cout << "directory_listing -> " << (conf->directory_listing ? "on" : "off") << std::endl;
        std::cout << std::endl ;
    }

    void    print_locations()
    {
        HashMap<std::string, t_location_configs *>::iterator it = dir_configs->begin();
        std::cout << CYAN_BOLD << "-- PRITING LOCATIONS --" << WHITE_BOLD << std::endl;
        while (it != dir_configs->end())
        {
            std::string dir = it->first;
            t_location_configs *conf = it->second; 
            std::cout << "location -> " << dir << std::endl;
            print_location_data(conf);
            it++;
        }
        std::cout << CYAN_BOLD << "-- END PRITING LOCATIONS --" << WHITE_BOLD << std::endl;
    }

    void    print_data()
    {
        print_server();
        print_locations();
    }

}   t_server ;