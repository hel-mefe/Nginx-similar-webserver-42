# include "../inc/http_handler.class.hpp"

void    HttpHandler::handle_http(t_client *client)
{
    int max_request_timeout = client->server->server_configs->max_request_timeout;

    if (client->state == READING_HEADER || client->state == WAITING)
    {
        int time_passed = time(NULL) - client->request_time;
        if (time_passed > max_request_timeout && client->state == READING_HEADER)
            client->state = SERVED;
        else
            client->state = (client->state == WAITING) ? READING_HEADER : WAITING;
        if (client->state == READING_HEADER && http_parser->read_header(client))
        {
            std::cout << RED_BOLD << "IS READING HEADER ..." << std::endl;
            if (!http_parser->parse_request(client)) // request is not well-formed
            {
                std::string bad_request = "HTTP/1.1 400 Bad Request\r\n\r\n";
                send(client->fd, bad_request.c_str(), sz(bad_request), 0);
                client->state = SERVED;
            }
            else
                architect_response(client);
        }
        else
        {
            std::cout << "WILL DISCONNECT DUE TO READ HEADER" << std::endl;
            if (time_passed > max_request_timeout && client->state == READING_HEADER)
                client->state = SERVED;
        }
    }
}

void    HttpHandler::handle_path_error(t_client *client, int code) // only called if code is exist in map
{
    t_request *req = client->request;
    t_response *res = client->response;
    std::map<int, std::string>          code_to_page;
    std::string                 error_page;
    std::string                 current_dir;

    if (res->dir_configs)
        code_to_page = res->dir_configs->code_to_page;
    else
        code_to_page = res->configs->code_to_page;
    error_page = (res->dir_configs) ? res->dir_configs->root : res->configs->root;
    if (error_page[sz(error_page) - 1] != '/')
        error_page += "/";
    error_page += code_to_page[code];
    if (!access(error_page.c_str(), R_OK))
    {
        client->state = SERVING_GET;
        set_root_file_path(client);
        current_dir = client->cwd;
        if (current_dir[sz(current_dir) - 1] != '/')
            res->filepath = current_dir + res->rootfilepath;
    }
    else
        client->state = SERVED;
    res->rootfilepath = get_cleanified_path(res->rootfilepath);
    res->filepath = get_cleanified_path(res->filepath);
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
    }
    if (write_it)
        res->write_response_in_socketfd(client->fd, false);
    // new_state = (connection == "keep-alive") ? KEEP_ALIVE : SERVED;
    // client->reset(new_state);
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

    if (!res->dir_configs)
        res->dir_configs = get_location_configs_from_path(client);
    if (!sz(res->directory_configs_path))
        res->directory_configs_path = get_longest_directory_prefix(client, req->path, true);
    std::cout << YELLOW_BOLD << "WE ARE DEALING WITH THIS LOCATION ===> " << res->directory_configs_path << std::endl;
    res->configs = client->server->server_configs;
    res->is_cgi = (req->extension == ".php" || req->extension == ".pl");
    if (res->is_cgi)
        res->cgi_path = res->configs->extension_cgi[res->extension];
    set_root_file_path(client); // needed for GET and DELETE
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
    rootfilepath = client->cwd + res->rootfilepath;
    if (!is_path_valid(rootfilepath))
    {
        std::cout << "[handle_file_path] -> handling file path 404" << std::endl;
        files_404 = (l_configs) ? l_configs->pages_404 : s_configs->pages_404;
        if (l_configs && IN_MAP(l_configs->code_to_page, 404))
            files_404.push_back(l_configs->code_to_page[404]);
        else if (IN_MAP(s_configs->code_to_page, 404))
            files_404.push_back(s_configs->code_to_page[404]);     
        std::string dirpath = get_longest_directory_prefix(client, rootfilepath, false);
        std::string p = client->cwd;
        p += (l_configs) ? dirpath : s_configs->root;
        if (!set_file_path(client->cwd, p, files_404))
        {
            res->rootfilepath = "";
            res->filepath = "";
        }
        else
            res->filepath = p;
        res->code = 404;
    }
    else
    {
        res->filepath = std::string(client->cwd) + res->rootfilepath;
        res->code = 200;
    }
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
    if (l_configs && IN_MAP(l_configs->code_to_page, 404))
        files_404.push_back(l_configs->code_to_page[404]);
    else
        files_404.push_back(s_configs->code_to_page[404]);
    rootfilepath = res->rootfilepath;
    std::string rootpath = s_configs->root;
    rootpath = (l_configs) ? rootpath + l_configs->root : rootpath;
    std::cout << GREEN_BOLD << "[HANDLING DIRECTORY WORKING ...]" << std::endl;
    if (!set_file_path(client->cwd, rootfilepath, indexes))
    {
        if (l_configs && l_configs->directory_listing)
        {
            res->code = 200;
            res->is_directory_listing = true;
            res->add("content-type", "text/html");
            res->add("transfer-encoding", "chunked");
            res->write_response_in_socketfd(res->fd, true);
            std::cout << GREEN_BOLD << "DIRECTORY LISTING IS ON!" << WHITE << std::endl;
        }
        else
        {
            if (set_file_path(client->cwd, rootpath, files_404))
            {
                res->rootfilepath = rootpath;
                res->filepath = client->cwd;
                if (res->filepath[sz(res->filepath) - 1] != '/')
                    res->filepath += "/";
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
        res->filepath = client->cwd + res->rootfilepath;
        res->code = 200;
    }
}



void    HttpHandler::architect_post_response(t_client *client)
{
    t_request    *req = client->request;
    t_response   *res = client->response;
    std::map<std::string, std::string>::iterator header = req->request_map.find("content-type");
    std::string url;

    url = client->cwd;
    url += res->rootfilepath;
    if (res->is_cgi)
        res->cgi_path = client->cwd + "/" + res->configs->extension_cgi[req->extension];
    res->filepath = client->cwd + res->rootfilepath;
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
    t_request *req;
    t_response *res;
    std::map<std::string, std::string>  *request_map;
    std::string connection;
    CLIENT_STATE    new_state;

    req = client->request;
    res = client->response;
    request_map = &req->request_map;

    req->is_file = (req->path[sz(req->path) - 1] != '/') ;
    req->print_data();
    set_response_configs(client);
    if ((handlers->handle_400(client) || handlers->handle_414(client) || handlers->handle_501(client) \
    || handlers->handle_413(client) || handlers->handle_405(client)) || handlers->handle_301(client))
        res->print_data();
    else
    {
        if (req->method == "GET")
            handlers->handle_200(client);
        else if (req->method == "DELETE")
            client->state = SERVING_DELETE;
        else if (req->method == "POST")
            architect_post_response(client);
    }

}