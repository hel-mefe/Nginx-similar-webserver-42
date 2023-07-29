#ifndef SERVER_HPP
# define SERVER_HPP

# include "header.hpp"
# include "structs.hpp"
# include "exceptions.hpp"
# include "multiplexer.interface.hpp"
# include "server.interface.hpp"

class Server
{
    private:
        t_server_configs                                        *server_configs;
        t_http_configs                                          *http_configs;
        HashMap<std::string, t_location_configs *>              *dir_configs; // directory configs (dir -> its configs)

    public:
        Server();
        Server& operator=(const Server&);
        Server(const Server&);
        ~Server();

        // Getters
        t_server_configs*   get_server_configs(){return server_configs;};
        t_http_configs*     get_http_configs(){return http_configs;}
        HashMap<std::string, t_location_configs *>* get_location_configs(){return dir_configs;}

        void                set_server_configs(t_server_configs *_server_configs);
        void                set_location_map(std::string location_name, t_location_configs *configs);
} ;

# endif