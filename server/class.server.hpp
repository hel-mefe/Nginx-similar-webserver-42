#ifndef SERVER_HPP
# define SERVER_HPP

# include "../includes/header.hpp"
# include "../includes/structures.hpp"
# include "../includes/exceptions.hpp"
# include "../includes/multiplexer.interface.hpp"
# include "../includes/server.interface.hpp"

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
        // void                print_data();

        // void    run_forked();
        // void    run_multiplexed();

        // // Multiplexers
        // void    build_with_kqueue();
        // void    build_with_select();
        // void    build_with_poll();

        // //Network functions
        // SOCKET  get_listener_socket(t_socket *listener);
        // void    read_request(t_socket *client);
        // void    add_connection(t_network *manager, SOCKET sockfd);
        // void    handle_socket(t_network *manager, int index);
        // void    add_write_socket(t_network *manager, t_socket *client);
    
        // //Method handlers
        // void    handle_get_request(int fd, t_request *req);
        // void    handle_post_request(int fd, t_request *req);
        // void    handle_delete_request(int fd, t_request *req);

        // // Handlers
        // void    disconnect_client(t_network *manager, int index);
        // void    serve_client(t_network *manager, int index);

        // void    serve_connected_clients(t_network *manager);
        // void    drop_timeout_connections(t_network *manager);
} ;

# endif