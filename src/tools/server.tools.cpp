# include "../includes/class.server.hpp"

Server::Server() : server_configs(0), http_configs(0), dir_configs(0)
{
    server_configs = new t_server_configs();
    // http_configs = new t_http_configs();
    dir_configs = new HashMap<std::string, t_location_configs*>();
}


Server::Server(const Server &s) : server_configs(0), http_configs(0), dir_configs(0)
{
    *this = s;
}

Server& Server::operator=(const Server &s)
{
    (void) s;
    return (*this);
}

Server::~Server()
{
    if (server_configs)
        delete server_configs;
    if (http_configs)
        delete http_configs;
    if (dir_configs)
        delete dir_configs;
}


void    Server::set_server_configs(t_server_configs *_server_configs)
{
    if (this->server_configs)
        delete this->server_configs;
    this->server_configs = _server_configs;
}

void    Server::set_location_map(std::string location_name, t_location_configs *configs)
{
    dir_configs->insert(std::make_pair(location_name, configs));
}