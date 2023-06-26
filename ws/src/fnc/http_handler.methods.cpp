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
        int time_passed = time(NULL) - client->request_time;
        if (time_passed > MAX_REQUEST_TIMEOUT)
            client->state = SERVED;
        else
            client->state = (client->state == WAITING) ? READING_HEADER : WAITING;
        if (client->state == READING_HEADER && http_parser->read_header(client))
        {
            http_parser->parse_request(client);
            architect_response(client);
            // alert("READ FALSE", YELLOW_BOLD);
            // if (client->state == WAITING && time(NULL) - client->request_time >= MAX_REQUEST_TIMEOUT)
            // client->state = SERVED ;
            return ;
        }
        // http_parser->parse_request(client);
        // architect_response(client);
    }       
}

bool    HttpHandler::handle_501(t_client *client)
{
    t_request *req = client->request;
    std::map<std::string, std::string>::iterator it = req->request_map.find("transfer-encoding");

    if (it != req->request_map.end())
    {
        if (it->second == "chunked")
            req->is_chunked = true;
        else
        {
            fill_response(client, 501, true);
            client->state = SERVED;
            return true ;
        }
    }
    return false ;
}

bool    HttpHandler::handle_400(t_client *client)
{
    t_request *req = client->request;
    std::map<std::string, std::string>  *request_map = &req->request_map;

    if ((req->method == "POST" && (!IN_MAP((*request_map), "content-type") || (!IN_MAP((*request_map), "transfer-encoding") && \
    !IN_MAP((*request_map), "content-length")))) || !is_request_uri_valid(req->path))
    {
        fill_response(client, 400, true);
        client->state = SERVED;
        return true ;
    }
    return false ;
}

bool HttpHandler::handle_414(t_client *client)
{
    t_request *req = client->request;
    std::map<std::string, std::string>  *request_map = &req->request_map;

    if (sz(req->path) >= MAX_REQUEST_URI_SIZE)
    {
        fill_response(client, 414, true);
        client->state = SERVED;
        return true ;
    }
    return false ;
}

bool    HttpHandler::handle_405(t_client *client)
{
    t_request *req;
    t_response *res;
    t_location_configs  *l_configs;
    t_server_configs    *s_configs;
    std::map<std::string, std::string>  *request_map;

    req = client->request;
    res = client->response;
    request_map = &req->request_map;
    l_configs = get_location_configs_from_path(client);
    s_configs = client->server->server_configs;
    std::vector<std::string>    allowed_methods = (l_configs) ? l_configs->allowed_methods : s_configs->allowed_methods;
    /*** Method is not allowed ****/
    if (std::find(allowed_methods.begin(), allowed_methods.end(), req->method) == allowed_methods.end())
    {
        fill_response(client, 405, true);
        client->state = SERVED;
        return true ;
    }
    return (false);
}

bool HttpHandler::handle_413(t_client *client)
{
    t_request *req = client->request;
    t_server_configs *sconf = client->server->server_configs;
    std::map<std::string, std::string>  *request_map;

    request_map = &req->request_map;
    if (IN_MAP((*request_map), "content-length"))
    {
        req->content_length = std::atoi(request_map->at("content-length").c_str());
        if(sconf->max_body_size < req->content_length)
        {
            fill_response(client, 413, true);
            client->state = SERVED;
            return true;
        }
    }
    return false;
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
    if (sz(res->filepath))
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
    std::string path;
    HashMap<std::string, t_location_configs*>   *dir_configs;
    t_location_configs  *d_configs;
    t_server_configs    *s_configs;

    req = client->request;
    res = client->response;
    request_map = &req->request_map;
    server = client->server;
    path = req->path;
    dir_configs = client->server->dir_configs;
    s_configs = client->server->server_configs;
    d_configs = get_location_configs_from_path(client);
    std::string pathr = (d_configs) ? d_configs->root : s_configs->root;
    pathr += path;
    if (path[sz(path) - 1] != '/' && is_directory_exist(pathr))
    {
        std::cout << CYAN_BOLD << "THE DIRECTORY EXIST AND THE REDIRECTION IS WORKING " << std::endl;
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
    /*** IN CASE OF FILE ***/
    // if (req->is_file)
    //     return false ;
    return (client->request->method == "GET" && set_redirection_path(client));
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

std::string HttpHandler::get_root_file_path(std::string &root, std::string &path, std::string &dirpath, bool sfile)
{
    int i = 0 ;
    std::string suffix;
    std::string res;
    std::cout << "--------------------------- " << std::endl;
    std::cout << "ROOT = " << root << std::endl;
    std::cout << "PATH = " << path << std::endl;
    std::cout << "DIR PATH = " << dirpath << std::endl;
    std::cout << "--------------------------- " << std::endl;
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
        {
            if (!sfile && ((sz(suffix) && suffix[sz(suffix) - 1] != '/') || !sz(suffix)))
                suffix = suffix + "/";
        }
        if (suffix[0] != '/')
            suffix = "/" + suffix;
        res = root + suffix;
    }
    else
    {
        if (sfile)
            res = "/" + root;
        else
            res = "/" + root + "/";
    }
    return res ;
}

std::string HttpHandler::get_clean_file_path(std::string root, std::string path, std::string dirpath, bool zfile)
{
    // root is the root /www for ex
    // path is the path in request /image.jpeg for example
    // dirpath is the directory path
    /** the resString = [from_start, pos_of_dirpath[ + rootstring ***/
    size_t pos = path.find_first_of(dirpath);
    std::string first_part = path.substr(0, pos);
    if (sz(root))
        first_part += root;
    else
        first_part += dirpath;
    std::string second_part = path.substr(pos + sz(dirpath));
    std::string res = first_part + second_part;
    if ((!zfile && sz(res) && res[sz(res) - 1] == '/') || (!sz(res)))
        res += "/";
    std::cout << "THE RESULT CLEAN FILE PATH ===> " << res << std::endl;
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
        std::cout << "ROOT IN LOCATION => " << root << std::endl;
        std::cout << "ROOT IN SERVER ==> " << s_configs->root << std::endl;
        std::cout << "REQ PATH => " << path << std::endl;
        res->rootfilepath = s_configs->root + get_clean_file_path(root, req->path, res->directory_configs_path, req->is_file);
        // if (req->is_file)
        //     res->rootfilepath = get_root_file_path(root, req->path, res->directory_configs_path, true);
        // else
        //     res->rootfilepath = get_root_file_path(root, req->path, res->directory_configs_path, false);
        std::cerr << "AFTER ROOT FILE PATH => " << res->rootfilepath << std::endl;
        // std::cout << YELLOW_BOLD << "DIRECTORY CONFIGS NEW ROOT PATH -> " << res->rootfilepath << WHITE << std::endl;
        // alert("DIRECTORY CONFIGS NEW ROOT PATH -> ", YELLOW_BOLD);
        // alert(res->rootfilepath, GREEN_BOLD);
    }
    else
    {
        root = s_configs->root;
        res->rootfilepath = root + req->path;
        // alert("SERVER CONFIGS NEW ROOT PATH -> ", YELLOW_BOLD);
        // alert(res->rootfilepath, GREEN_BOLD);
    }
}

void    HttpHandler::set_configurations(t_client *client)
{
    t_request *req = client->request;
    t_response *res = client->response;

    res->dir_configs = get_location_configs_from_path(client);
    res->directory_configs_path = get_longest_directory_prefix(client, req->path, true);
    std::cout << YELLOW_BOLD << "WE ARE DEALING WITH THIS LOCATION ===> " << res->directory_configs_path << std::endl;
    res->configs = client->server->server_configs;
    res->is_cgi = (req->extension == ".php" || req->extension == ".pl");
    if (res->is_cgi)
        res->cgi_path = res->configs->extension_cgi[res->extension];
    set_root_file_path(client); // needed for GET and DELETE
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
    path = s_configs->root + req->path;
    fullpath = current_directory + "/" + path;
    std::cout << GREEN_BOLD << "[THE FULL PATH IN LOCATIONS] ==> " << fullpath << std::endl;
    if (req->is_file && !is_path_valid(fullpath))
    {
        std::cout << "[handle_locations] -> THE PATH IS NOT VALID AND WORKING WITH 404" << std::endl;
        files_404 = (l_configs) ? l_configs->pages_404 : s_configs->pages_404;        
        std::string rootpath = l_configs ? l_configs->root : s_configs->root;
        if (set_file_path(rootpath, files_404))
        {
            client->state = SERVING_GET;
            res->rootfilepath = rootpath;
        }
        else
            client->state = SERVED;
        res->filepath = current_directory + res->rootfilepath;
        res->filename = req->path;
        req->extension = get_extension(res->filepath);
        res->extension = req->extension;
        fill_response(client, 404, true);
        return true ;
    }
    else if (is_redirection(client)) // also sets the redirecto_to
    {
        std::cout << "REDIREEEECTION ...." << std::endl;
        fill_response(client, 301, true);
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
    rootfilepath = std::string(getwd(NULL)) + res->rootfilepath;
    if (!is_path_valid(rootfilepath))
    {
        std::cout << "[handle_file_path] -> handling file path 404" << std::endl;
        files_404 = (l_configs) ? l_configs->pages_404 : s_configs->pages_404;
        std::string dirpath = get_longest_directory_prefix(client, rootfilepath, false);
        std::string p = getwd(NULL);
        p += (l_configs) ? dirpath : s_configs->root;
        // std::cout << WHITE_BOLD << "Longest directory prefix -> " << dirpath << WHITE << std::endl;
        if (!set_file_path(p, files_404))
        {
            res->rootfilepath = "";
            res->filepath = "";
            // std::cout << YELLOW_BOLD << "FILES 404 WERE NOT FOUND!" << WHITE << std::endl;
        }
        else
            res->filepath = p;
        res->code = 404;
    }
    else
    {
        res->filepath = std::string(getwd(NULL)) + res->rootfilepath;
        res->code = 200;
    }
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
    std::string rootpath = s_configs->root;
    rootpath = (l_configs) ? rootpath + l_configs->root : rootpath;
    std::cout << GREEN_BOLD << "[HANDLING DIRECTORY WORKING ...]" << std::endl;
    if (!set_file_path(rootfilepath, indexes))
    {
        if (l_configs && l_configs->directory_listing)
        {
            res->code = 200;
            res->is_directory_listing = true;
            res->add("content-type", "text/html");
            std::cout << GREEN_BOLD << "DIRECTORY LISTING IS ON!" << WHITE << std::endl;
        }
        else
        {
            if (set_file_path(rootpath, files_404))
            {
                res->rootfilepath = rootpath;
                res->filepath = std::string(getwd(NULL));
                res->filepath += res->rootfilepath[0] == '/' ? res->rootfilepath.substr(1) : res->rootfilepath;
                std::cout << PURPLE_BOLD << res->rootfilepath  << WHITE << std::endl;
            }
            else
            {
                res->rootfilepath = "";
                res->filepath = "";
            }
            res->code = 404;
        }
    }
    else
    {
        res->rootfilepath = rootfilepath;
        res->filepath = std::string(getwd(NULL)) + res->rootfilepath;
        res->code = 200;
    }
}

void    HttpHandler::cleanify_response(t_response *res)
{
    std::string filepath = res->filepath;
    std::string rootfilepath = res->rootfilepath;
    std::list<char> slist;
    bool    flag = false;

    for (int i = 0; i < sz(filepath); i++)
    {
        if (!flag || (flag && filepath[i] != '/'))
            slist.push_back(filepath[i]);
        flag = (filepath[i] == '/'); 
    }
    char    *s = new char[sz(slist) + 1];
    int i = 0;
    for (auto x: slist)
        s[i++] = x;
    s[i] = 0;
    res->filepath = s;
    delete[] s;
    slist.clear();
    flag = false ;
    for (int i = 0; i < sz(rootfilepath); i++)
    {
        if (!flag || (flag && rootfilepath[i] != '/'))
            slist.push_back(rootfilepath[i]);
        flag = (rootfilepath[i] == '/'); 
    }
    s = new char[sz(slist) + 1];
    i = 0;
    for (auto x: slist)
        s[i++] = x;
    s[i] = 0;
    res->rootfilepath = s;
    slist.clear();
    delete[] s;
}

void    HttpHandler::architect_get_response(t_client *client)
{
    t_request           *req = client->request;
    t_response          *res = client->response;
    t_server_configs    *s_configs;
    t_location_configs  *l_configs;

    s_configs = res->configs;
    l_configs = res->dir_configs;
    if (req->is_file) // file
        handle_file_path(client);
    else // directory
        handle_directory_path(client);

    if (sz(res->filepath) && access(res->filepath.c_str(), R_OK)) // forbidden
    {
        fill_response(client, 403, true);
        client->state = SERVED ;
    }
    else // Access allowed
    {
        std::cout << GREEN_BOLD << " -- WE ARE DEALING WITH THIS FILE PATH ==> " << res->filepath << std::endl;
        res->extension = get_extension(res->filepath);
        res->is_cgi = IN_MAP((s_configs->extension_cgi), res->extension);
        if (res->is_cgi)
            res->cgi_path = std::string(getwd(NULL)) + "/" + s_configs->extension_cgi[res->extension];
        fill_response(client, res->code, true);
        client->state = (sz(res->filepath) || res->is_directory_listing ? SERVING_GET : SERVED);
    }
    cleanify_response(res);
    std::cout << "CLEANIFIED FILEPATH ==> " << res->filepath << std::endl;
    std::cout << "CLEANIFIED ROOTFILEPATH ==> " << res->rootfilepath << std::endl;
}

void    HttpHandler::architect_delete_response(t_client *client)
{
    std::cout << RED_BOLD << "ARHICTECTURING DELETE RESPONSE ..." << WHITE << std::endl;
    client->state = SERVING_DELETE;
}

void    HttpHandler::architect_post_response(t_client *client)
{
    t_request    *req = client->request;
    t_response   *res = client->response;
    std::map<std::string, std::string>::iterator header = req->request_map.find("content-type");
    std::string url = getwd(NULL) + res->rootfilepath;

    if (res->is_cgi)
        res->cgi_path = std::string(getwd(NULL)) + "/" + res->configs->extension_cgi[req->extension];
    res->filepath = std::string(getwd(NULL)) + res->rootfilepath;
    if (access(url.c_str(), F_OK))
    { 
        fill_response(client, 404, true);
        client->state = SERVED;
        return;
    }
    if (!res->dir_configs->upload)
    {
        fill_response(client, 403, true);
        client->state = SERVED;
        return;
    }
    client->state = SERVING_POST;
    for (std::map<std::string, std::string>::iterator it = mimes->begin(); it != mimes->end(); it++)
    {
        if (it->second == header->second)
        {
            req->extension = it->first;
            return;
        }
    }
    req->extension = ".txt";
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

    // if (req->path[sz(req->path) - 1] != '/' && !req->is_file)
    //     req->path += "/";
    req->is_file = (req->path[sz(req->path) - 1] != '/') ;
    req->print_data();
    if ((handle_400(client) || handle_414(client) || handle_501(client) || handle_413(client) || handle_405(client)) || handle_locations(client))
        res->print_data();
    else
    {
        set_configurations(client);
        if (req->method == "GET")
            architect_get_response(client);
        else if (req->method == "DELETE")
            client->state = SERVING_DELETE;
        else if (req->method == "POST")
            architect_post_response(client);
    }
}