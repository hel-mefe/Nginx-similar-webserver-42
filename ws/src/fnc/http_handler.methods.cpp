# include "../inc/http_handler.class.hpp"

void    HttpHandler::handle_http(t_client *client)
{
    if (client->state == KEEP_ALIVE)
        return ;
    else if (client->state == READING_HEADER || client->state == WAITING)
    {
        // if (client->state == WAITING && time(NULL) - client->request_time >= MAX_REQUEST_TIMEOUT)
        // {
        //     alert("TIMEOUT BLOCK", YELLOW_BOLD);
        //     client->state = SERVED;
        // }
        if (http_parser->read_header(client))
        {
            http_parser->parse_request(client);
            architect_response(client);
            // alert("READ FALSE", YELLOW_BOLD);
            // if (client->state == WAITING && time(NULL) - client->request_time >= MAX_REQUEST_TIMEOUT)
            // client->state = SERVED ;
            return ;
        }
        int time_passed = time(NULL) - client->request_time;
        if (time_passed > MAX_REQUEST_TIMEOUT)
            client->state = SERVED;
        // http_parser->parse_request(client);
        // architect_response(client);
    }       
}

bool    HttpHandler::handle_501(t_client *client)
{
    t_request *req;
    t_response *res;
    std::map<std::string, std::string>  *request_map;
    std::string connection;
    CLIENT_STATE    new_state;

    req = client->request;
    res = client->response;
    request_map = &req->request_map;
    if (IN_MAP((*request_map), "transfer-encoding") && request_map->at("transfer-encoding") != "chunked")
    {
        fill_response(client, 501, true);
        return true ;
    }
    return false ;
}

bool    HttpHandler::handle_400(t_client *client)
{
    t_request *req;
    t_response *res;
    std::map<std::string, std::string>  *request_map;
    std::string connection;
    CLIENT_STATE    new_state;

    req = client->request;
    res = client->response;
    request_map = &req->request_map;
    if ((!IN_MAP((*request_map), "transfer-encoding") && \
    !IN_MAP((*request_map), "content-length") && req->method == "POST") \
    || !is_request_uri_valid(req->path))
    {
        fill_response(client, 400, true);
        return true ;
    }
    return false ;
}

bool HttpHandler::handle_414(t_client *client)
{
    t_request *req;
    t_response *res;
    std::map<std::string, std::string>  *request_map;
    std::string connection;
    CLIENT_STATE    new_state;

    req = client->request;
    res = client->response;
    request_map = &req->request_map;
    if (sz(req->path) >= MAX_REQUEST_URI_SIZE)
    {
        fill_response(client, 414, true);
        return true ;
    }
    return false ;
}

bool HttpHandler::handle_413(t_client *client)
{
    t_request *req;
    t_response *res;
    std::map<std::string, std::string>  *request_map;
    std::string connection;
    CLIENT_STATE    new_state;

    req = client->request;
    res = client->response;
    request_map = &req->request_map;
    if (IN_MAP((*request_map), "content-length"))
    {
        int cl = std::atoi(request_map->at("content-length").c_str());
        if(client->server->server_configs->max_body_size * 1000000 < cl)
        {
            fill_response(client, 413, true);
            return true;
        }
    }
    return false ;
}

void    HttpHandler::fill_response(t_client *client, int code, bool write_it)
{
    t_request *req;
    t_response *res;
    std::map<std::string, std::string>  *request_map;
    std::string connection;
    CLIENT_STATE    new_state;

    req = client->request;
    res = client->response;
    request_map = &req->request_map;
    std::cout << RED_BOLD << code << " -> " << codes->at(code) << WHITE << std::endl;
    res->http_version = HTTP_VERSION;
    res->status_code = std::to_string(code);
    res->status_line = codes->at(code);
    connection = req->get_param("connection"); // used for keep-alive
    if (sz(connection)) // type of connection
        res->add("connection", "closed");
    if (sz(res->redirect_to)) // redirection exist
        res->add("location", res->redirect_to);
    if (sz(res->rootfilepath))
    {
        std::string ext = res->extension;
        std::string ctype = IN_MAP((*mimes), ext) ? mimes->at(ext) : "text/plain";
        res->add("content-type", ctype);
        res->add("transfer-encoding", "chunked");
        res->add("connection", "closed");
        if (!res->is_cgi) // dealing with file
            res->fd = open(res->filepath.c_str(), O_RDONLY); // for sure valid since I checked it before
        else
            std::cout << GREEN_BOLD << "***** DEALING WITH CGI ****" << std::endl ;
        // else // dealing with cgi
        //     pipe(res->cgi_pipe);
    }
    if (write_it)
        res->write_response_in_socketfd(client->fd);
    // new_state = (connection == "keep-alive") ? KEEP_ALIVE : SERVED;
    // client->reset(new_state);
}

bool    HttpHandler::set_redirection_path(t_client *client)
{
    t_request *req;
    t_response *res;
    t_server    *server;
    std::map<std::string, std::string>  *request_map;
    std::string connection;
    std::string path;
    CLIENT_STATE    new_state;
    HashMap<std::string, t_location_configs*>   *dir_configs;

    req = client->request;
    res = client->response;
    request_map = &req->request_map;
    server = client->server;
    path = req->path;
    dir_configs = client->server->dir_configs;
    if (path[sz(path) - 1] != '/')
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
                std::cout << PURPLE_BOLD << "REQUESTED PATH = " << rpath << " - redirected to = " << res->redirect_to << std::endl;
                return true ;
            }
        }
    }
    return false ;
}

bool    HttpHandler::is_redirection(t_client *client)
{
    t_request *req;
    t_response *res;
    t_server    *server;
    std::map<std::string, std::string>  *request_map;
    std::string connection;
    CLIENT_STATE    new_state;

    req = client->request;
    res = client->response;
    request_map = &req->request_map;
    server = client->server;
    if (req->is_file)
        return false ;
    return (req->method == "GET" && set_redirection_path(client));
}

bool    HttpHandler::is_method_valid(std::string &method)
{
    return (method == "POST" || method == "GET" || method == "DELETE");
}

t_location_configs  *HttpHandler::get_location_configs_from_path(t_client *client)
{
    t_request *req;
    t_response *res;
    HashMap<std::string, t_location_configs *> *dir_configs;
    std::string path;

    req = client->request;
    res = client->response;
    dir_configs = client->server->dir_configs;
    path = req->path;
    for (int i = sz(path) - 1; i >= 0; i--)
    {
        if (path[i] == '/')
        {
            std::string rpath = path.substr(0, i + 1);
            if (IN_MAP((*dir_configs), rpath))
            {
                std::cout << GREEN_BOLD << "LOCATION WAS FOUND -> " << rpath << WHITE << std::endl;
                return dir_configs->at(rpath);
            }
        }
    }
    std::cout << RED_BOLD << "LOCATION WAS NOT FOUND!" << WHITE << std::endl;
    return nullptr ;
}

std::string HttpHandler::get_root_file_path(std::string &root, std::string &path, std::string &dirpath)
{
    int i = 0 ;
    std::string suffix;
    std::string res;

    for (; i < sz(path) && i < sz(dirpath) && path[i] == dirpath[i]; i++)
        i++ ;
    if (i < sz(path))
    {
        if (path[i] == '/')
            i--;
        while (i >= 0 && path[i] != '/')
            i--;
        if (i < sz(path))
            suffix = path.substr(i, sz(path) - i);
        if (suffix[sz(suffix) - 1] != '/')
            suffix = suffix + "/";
        if (suffix[0] != '/')
            suffix = "/" + suffix;
        res = root + suffix;
    }
    else
        res = "/" + root + "/";
    return res ;
}

void    HttpHandler::set_root_file_path(t_client *client)
{
    t_location_configs *d_configs;
    t_server_configs   *s_configs;
    t_request          *req;
    t_response         *res;
    std::string        path;
    std::string        root;

    req = client->request;
    res = client->response;
    d_configs = client->response->dir_configs;
    s_configs = client->server->server_configs;
    path = req->path;
    if (d_configs)
    {
        root = d_configs->root;
        res->rootfilepath = get_root_file_path(root, req->path, res->directory_configs_path);
        std::cout << YELLOW_BOLD << "DIRECTORY CONFIGS NEW ROOT PATH -> " << res->rootfilepath << WHITE << std::endl;
    }
    else
    {
        root = s_configs->root;
        res->rootfilepath = root + req->path;
        alert("SERVER CONFIGS NEW ROOT PATH -> ", YELLOW_BOLD);
        alert(res->rootfilepath, GREEN_BOLD);
    }
}

void    HttpHandler::set_configurations(t_client *client)
{
    t_request *req;
    t_response *res;
    HashMap<std::string, t_location_configs *> *dir_configs;

    req = client->request;
    res = client->response;
    res->dir_configs = get_location_configs_from_path(client);
    res->directory_configs_path = get_longest_directory_prefix(client, req->path, true);
    res->configs = client->server->server_configs;
    res->is_cgi = (res->extension == ".php");
    set_root_file_path(client); // needed for GET and DELETE
    // std::cout << "AFTER SETTING THE ROOT" << std::endl;
    // if (res->dir_configs)
    //     std::cout << YELLOW_BOLD << "WORKING WITH DIRECTORY CONFIGS!" << WHITE << std::endl;
    // else
    //     std::cout << YELLOW_BOLD << "WORKING WITH SERVER CONFIGS!" << WHITE << std::endl; 
}

bool    HttpHandler::handle_locations(t_client *client)
{
    t_request                   *req;
    t_response                  *res;
    std::map<std::string, std::string>  *request_map;
    std::string                 connection;
    std::string                 current_directory = getwd(NULL);
    std::string                 path;
    std::string                 fullpath;
    CLIENT_STATE                new_state;
    std::vector<std::string>    files_404;
    t_location_configs          *l_configs;
    t_server_configs            *s_configs;

    req = client->request;
    res = client->response;
    request_map = &req->request_map;
    l_configs = get_location_configs_from_path(client);
    s_configs = client->server->server_configs;
    path = req->path;
    fullpath = std::string(getwd(NULL)) + path;
    if (req->is_file && !is_path_valid(fullpath))
    {
        files_404 = (l_configs) ? l_configs->pages_404 : s_configs->pages_404;        
        std::string rootpath = l_configs ? l_configs->root + req->path : s_configs->root + req->path;
        if (set_file_path(rootpath, files_404))
        {
            client->state = SERVING_GET;
            res->rootfilepath = rootpath;
        }
        else
            client->state = SERVED;
        res->filepath = current_directory + res->rootfilepath;
        res->filename = req->path;
        res->extension = req->extension;
        fill_response(client, 404, true);
        return true ;
    }
    else if (is_redirection(client)) // also sets the redirecto_to
    {
        fill_response(client, 301, true);
        return true ;
    }
    else if (!is_method_valid(req->method))
    {
        fill_response(client, 405, true);
        return true ;
    }
    return false ;
}


void    HttpHandler::handle_file_path(t_client *client)
{
    t_request                   *req;
    t_response                  *res;
    t_server_configs            *s_configs;
    t_location_configs          *l_configs;
    std::string                 rootfilepath;
    std::vector<std::string>    files_404;   

    req = client->request;
    res = client->response;
    s_configs = res->configs;
    l_configs = res->dir_configs;
    rootfilepath = res->rootfilepath;
    if (!is_path_valid(rootfilepath))
    {
        files_404 = (l_configs) ? l_configs->pages_404 : s_configs->pages_404;
        std::string dirpath = get_longest_directory_prefix(client, rootfilepath, false);
        // std::cout << WHITE_BOLD << "Longest directory prefix -> " << dirpath << WHITE << std::endl;
        if (!set_file_path(dirpath, files_404))
        {
            res->rootfilepath = "";
            res->filepath = "";
            // std::cout << YELLOW_BOLD << "FILES 404 WERE NOT FOUND!" << WHITE << std::endl;
        }
        res->code = 404;
    }
    else
        res->code = 200;
//     std::string fullpath = (l_configs) ? l_configs->root + req->path : s_configs->root + req->path;
//     if (is_path_valid(fullpath))
//         return ;
//     set_404_file(client);
    // std::cout << CYAN_BOLD << "HANDLING FILE PATH IS RUNNING ..." << std::endl;
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
        std::string fullpath = std::string(getwd(NULL));
        ipath = fullpath + ipath;
        if (is_path_valid(ipath))
        {
            res->filename = req->path + indexes[i];
            res->filepath = ipath;
            res->extension = get_extension(res->filename);
            // std::cout << YELLOW_BOLD << "INDEX FOUND = " << res->filename << std::endl;
            // std::cout << "FULLPATH = " << res->filepath << std::endl;
            // std::cout << "EXTENSION = " << res->extension << WHITE << std::endl;
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

void    HttpHandler::handle_directory_path(t_client *client)
{
    t_request                   *req;
    t_response                  *res;
    t_server_configs            *s_configs;
    t_location_configs          *l_configs;
    std::vector<std::string>    indexes;
    std::vector<std::string>    files_404;
    std::string                 rootfilepath;
    std::string                 path;
    std::string                 cwd;

    req = client->request;
    res = client->response;
    s_configs = res->configs;
    l_configs = res->dir_configs;
    indexes = (l_configs ? l_configs->indexes : s_configs->indexes);
    files_404 = (l_configs ? l_configs->pages_404 : s_configs->pages_404);
    rootfilepath = res->rootfilepath;
    if (!set_file_path(rootfilepath, indexes))
    {
        // std::cout << YELLOW_BOLD << "INDEXES WERE NOT SET FOR DIRECTORY!" << WHITE << std::endl;
        if (l_configs && l_configs->directory_listing)
            std::cout << GREEN_BOLD << "DIRECTORY LISTING IS ON!" << WHITE << std::endl;
        else
            std::cout << RED_BOLD << "DIRECTORY LISTING IS OFF!" << WHITE << std::endl;
        if (set_file_path(rootfilepath, files_404))
        {
            // alert("PAGES 404 WERE SET", YELLOW_BOLD);
            res->rootfilepath = rootfilepath;
            res->filepath = std::string(getwd(NULL)) + res->rootfilepath;
            std::cout << PURPLE_BOLD << res->rootfilepath  << WHITE << std::endl;
        }
        else
        {
            res->rootfilepath = "";
            res->filepath = "";
        }
        res->code = 404;
    }
    else
    {
        res->rootfilepath = rootfilepath;
        res->filepath = std::string(getwd(NULL)) + res->rootfilepath;
        res->code = 200;
    }
}


void    HttpHandler::architect_get_response(t_client *client)
{
    t_request           *req;
    t_response          *res;
    t_server_configs    *s_configs;
    t_location_configs  *l_configs;

    // std::cout << RED_BOLD << "ARHICTECTURING GET RESPONSE ..." << WHITE << std::endl;
    req = client->request;
    res = client->response;
    s_configs = res->configs;
    l_configs = res->dir_configs;
    if (req->is_file) // file
    {
        // std::cout << GREEN_BOLD << "FILE PATH HANDLING IS RUNNING ... " << WHITE << std::endl;
        handle_file_path(client);
    }
    else // directory
    {
        // std::cout << GREEN_BOLD << "DIRECTORY PATH HANDLING IS RUNNING ..." << WHITE << std::endl;
        handle_directory_path(client);
    }

    if (sz(res->filepath) && access(res->filepath.c_str(), R_OK)) // forbidden
    {
        // std::cout << RED_BOLD << "ACCESS DENIED!" << std::endl; 

        fill_response(client, 403, true);
        client->state = SERVED ;

    }
    else // Access allowed
    {
        // std::cout << GREEN_BOLD << "ACCESS ALLOWED!" << std::endl;
        res->extension = get_extension(res->filepath);
        res->is_cgi = IN_MAP((s_configs->extension_cgi), res->extension);
        if (res->is_cgi)
            res->cgi_path = std::string(getwd(NULL)) + "/" + s_configs->extension_cgi[res->extension];
        fill_response(client, res->code, true);
        client->state = (sz(res->filepath) ? SERVING_GET : SERVED);
    }

    // std::cout << WHITE_BOLD << "PATH => " << res->filepath << WHITE << std::endl;
}

void    HttpHandler::architect_delete_response(t_client *client)
{
    std::string path = std::string(getwd(NULL)) + client->response->rootfilepath;
    std::cout << RED_BOLD << "ARHICTECTURING DELETE RESPONSE ..." << WHITE << std::endl;
    client->state = SERVING_DELETE;
}

void    HttpHandler::architect_post_response(t_client *client)
{
    std::map<std::string, std::string> *map = &client->request->request_map;
    client->state = SERVING_POST;
    if (!client->request->extension.empty()) return;
    std::map<std::string, std::string>::iterator header = map->find("content-type");
    if (header == map->end()) {fill_response(client, 400, true); client->state = SERVED; return;}
    for (std::map<std::string, std::string>::iterator it = mimes->begin(); it != mimes->end(); it++)
    {
        if (it->second == header->second)
        {
            client->request->extension.append(it->first);
            return;
        }
    }
    client->request->extension.append(".txt");
}

void    HttpHandler::architect_response(t_client *client)
{
    t_request *req;
    t_response *res;
    std::map<std::string, std::string>  *request_map;
    std::string connection;
    CLIENT_STATE    new_state;

    req = client->request;
    res = client->response;
    request_map = &req->request_map;

    req->print_data();
    if (handle_400(client) || handle_414(client) || handle_501(client) || handle_413(client) || (req->method == "GET" && handle_locations(client)))
        res->print_data();
    else
    {
        set_configurations(client);
        if (req->method == "GET")
            architect_get_response(client);
        else if (req->method == "DELETE")
            architect_delete_response(client);
        else if (req->method == "POST")
            architect_post_response(client);
    }
}