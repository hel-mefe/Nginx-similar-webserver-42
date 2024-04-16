# include "../includes/http_handler.class.hpp"



void    HttpHandler::handle_http(t_client *client)
{
    t_server_configs *s_configs = client->server->server_configs;
    int max_request_timeout = (client->state == KEEP_ALIVE ? s_configs->keep_alive_timeout : s_configs->max_request_timeout);
    int time_passed = time(NULL) - client->request_time;
    CLIENT_STATE      prev_state = client->state;

    if (client->state == WAITING || client->state == KEEP_ALIVE)
    {
        if (time_passed > max_request_timeout)
            client->state = SERVED;
        else
            client->state = READING_HEADER;
    }
    if (client->state == READING_HEADER)
    {
        if (http_parser->read_header(client))
        {
            int code = http_parser->parse_request(client);
            if (code != 0) // request is not well-formed
            {
                std::string ress = "HTTP/1.1 ";
                ress += std::to_string(code) + " " + codes->at(code) + "\r\nContent-length: 0\r\n\r\n";
                send(client->fd, ress.c_str(), sz(ress), 0);
                client->state = SERVED;
            }
            else
                architect_response(client);
        }
        else
            client->state = client->state != SERVED ? prev_state : SERVED; // in case http_parser read 0 characters then connection is closed
    }
}

bool    HttpHandler::set_redirection_path(t_client *client)
{
    t_request *req;
    t_response *res;
    std::string path;
    HashMap<std::string, t_location_configs*>   *dir_configs;
    t_location_configs  *d_configs;
    t_server_configs    *s_configs;

    req = client->request;
    res = client->response;
    path = req->path;
    dir_configs = client->server->dir_configs;
    s_configs = client->server->server_configs;
    d_configs = get_location_configs_from_path(client);
    std::string pathr = (d_configs) ? d_configs->root : s_configs->root;
    pathr += path;
    if (path[sz(path) - 1] != '/' && is_directory_exist(client->cwd, pathr))
    {
        res->redirect_to = path + "/";
        return true ;
    }
    for (int i = sz(path); i >= 0; i--)
    {
        if (path[i] == '/')
        {
            std::string rpath;

            rpath = path.substr(0, i + 1);
            if (IN_MAP((*dir_configs), rpath) && \
            sz(dir_configs->at(rpath)->redirection))
            {
                res->redirect_to = dir_configs->at(rpath)->redirection;
                return true ;
            }
        }
    }
    return false ;
}

bool    HttpHandler::is_method_valid(std::string &method)
{
    return (method == "POST" || method == "GET" || method == "DELETE");
}

t_location_configs  *HttpHandler::get_location_configs_from_path(t_client *client)
{
    t_request *req;
    HashMap<std::string, t_location_configs *> *dir_configs;
    std::string path;

    req = client->request;
    dir_configs = client->server->dir_configs;
    path = req->path;
    for (int i = sz(path) - 1; i >= 0; i--)
    {
        if (path[i] == '/')
        {
            std::string rpath = path.substr(0, i + 1);
            if (IN_MAP((*dir_configs), rpath))
                return dir_configs->at(rpath);
        }
    }
    return nullptr ;
}

bool    HttpHandler::set_directory_indexes(t_client *client) // takes directory and sets files 
{
    t_request                   *req;
    t_response                  *res;
    t_server_configs            *s_configs;
    t_location_configs          *l_configs;
    std::vector<std::string>    indexes;

    req = client->request;
    res = client->response;
    s_configs = res->configs;
    l_configs = res->dir_configs;
    indexes = (l_configs) ? l_configs->indexes : s_configs->indexes;
    for (int i = 0; i < sz(indexes); i++)
    {
        std::string ipath = req->path + indexes[i];
        std::string fullpath = client->cwd;
        ipath = fullpath + ipath;
        if (is_path_valid(ipath))
        {
            res->filename = req->path + indexes[i];
            res->filepath = ipath;
            res->extension = get_extension(res->filename);
            return true;
        }
    }
    return false;
}


std::string HttpHandler::get_longest_directory_prefix(t_client *client, std::string path, bool is_config)
{
    std::string longest_prefix;
    HashMap<std::string, t_location_configs *> *dir_configs;

    dir_configs = client->server->dir_configs;
    for (int i = sz(path) - 1; i >= 0; i--)
    {
        if (path[i] == '/')
        {
            longest_prefix = path.substr(0, i + 1);
            if (!is_config || \
            (is_config && IN_MAP((*dir_configs), longest_prefix)))
                return longest_prefix ;
        }
    }
    return "";
}
/**
 * - responsible for setting configs and dir_configs
 * - responsible for setting directory_configs_path
*/
void    HttpHandler::set_response_configs(t_client *client)
{
    t_response  *res;
    t_request   *req;

    res = client->response;
    req = client->request;
    res->dir_configs = get_location_configs_from_path(client);
    res->directory_configs_path = get_longest_directory_prefix(client, req->path, true);
    res->configs = client->server->server_configs;
    res->root = res->dir_configs ? res->dir_configs->root : res->configs->root;
}

void    HttpHandler::architect_response(t_client *client)
{
    t_request *req = client->request;
    t_response *res = client->response;

    set_response_configs(client);
    req->is_file = (req->path[sz(req->path) - 1] != '/');
    if (req->method == "OPTIONS")
        client->state = SERVING_OPTIONS;
    else if (req->method == "TRACE")
        client->state = SERVING_TRACE;
    else if ((handlers->handle_400(client) || handlers->handle_414(client) || handlers->handle_501(client) \
    || handlers->handle_413(client) || handlers->handle_405(client)) || handlers->handle_301(client))
        return ;
    else
    {
        handlers->change_path(client);
        if (req->method == "GET" || req->method == "HEAD")
            handlers->handle_200(client);
        else if (req->method == "DELETE")
            client->state = SERVING_DELETE;
        else if (req->method == "POST")
        {
            std::string header = req->request_map.at("content-type");
            res->extension = get_extension(req->path);
            res->is_cgi = IS_CGI_EXTENSION(res->extension);
            if(res->is_cgi)
            {
                res->cgi_path = res->configs->extension_cgi[res->extension];
                client->state = SERVING_CGI;
                return;
            }
            client->state = SERVING_POST;
            for (std::map<std::string, std::string>::iterator it = mimes->begin(); it != mimes->end(); it++)
            {
                if (it->second == header)
                {
                    req->extension = it->first;
                    return;
                }
            }
            req->extension = ".txt";
        }
        else if (req->method == "PUT")
            client->state = SERVING_PUT;
    }
}