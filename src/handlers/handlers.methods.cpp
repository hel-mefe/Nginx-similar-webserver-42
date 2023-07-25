# include "../includes/class.handlers.hpp"

/***
 * this file includes all the handling methods
 * each function do one work which is handling the code response
 * functions are declared as follows handle_[code] ex. handle_404, handle_200
 * all the functions take one parameter which is the client struct
 * the documentation is written for each function above it
*/

void    Handlers::fill_response(t_client *client, int code, bool write_it)
{
    t_response *res;
    std::string connection;

    res = client->response;
    res->http_version = HTTP_VERSION;
    res->status_code = std::to_string(code);
    res->status_line = codes->at(code);
    std::cout << CYAN_BOLD << code << " : " << res->status_line << WHITE << std::endl;
    if (IN_MAP(client->request->request_map, "connection") && client->request->request_map["connection"] == "keep-alive")
        res->add("connection", "keep-alive");
    if (sz(res->redirect_to)) // redirection exists
        res->add("location", res->redirect_to);
    if (sz(res->filepath) || res->is_directory_listing)
    {
        std::string ext = res->extension;
        std::string ctype = IN_MAP((*mimes), ext) ? mimes->at(ext) : "text/plain";
        if (res->is_directory_listing)
            ctype = "text/html";
        res->add("content-type", ctype);
        if (!res->is_cgi) // dealing with file
            res->fd = open(res->filepath.c_str(), O_RDONLY); // for sure valid since I checked it before
        if (IN_MAP(client->request->request_map, "transfer-encoding") && client->request->request_map["transfer-encoding"] == "chunked")
            res->is_chunked = true;
        else
            res->is_chunked = false;
    }
    if (write_it)
    {
        add_to_logs(client);
        if (client->request->method == "GET")
        {
            if (sz(res->filepath))
                std::cout << "file path is true: " << res->filepath << std::endl ;
            res->write_response_in_socketfd(client->fd);
        }
        else if (client->request->method == "HEAD")
        {
            res->filepath = ""; // set to empty string therefore \r\n\r\n can be sent
            res->write_response_in_socketfd(client->fd);
        }
        else
            res->write_response_in_socketfd(client->fd);
    }
}

/** this function is used to change the path from its state to another state depending on the root provided
 * let's suppose we have a path like this /dir1/dir2 and also let's suppose that /dir1 is defined in the config file 
 * also the root for dir1 is (root = justRootExample)
 * then the path should become as follows /justRootExample/dir2
 * we took out the /dir1 and replace it with /justRootExample that's what this function does
 */
std::string Handlers::get_path_after_change(t_client *client, std::string root)
{
    t_request   *req;
    t_response  *res;
    std::string p;
    std::string resp;
    size_t      first_occ;

    req = client->request;
    res = client->response;
    p = req->path;
    first_occ = p.find_first_of(res->directory_configs_path);
    if (sz(root))
        resp = p.substr(0, first_occ) + "/" + root + "/" + p.substr(first_occ + sz(res->directory_configs_path));
    else
        resp = p;
    resp = get_cleanified_path(resp);
    return (resp);
}

/***
 * - same as above function, this is kind of the parent function that calls the function above
 * - this function handles that in the client struct which means it handles the client struct
 * - meanwhile the function above takes just a string and does what necessary depending on the configs
 * provided in both the server configs and the location configs
***/

bool    Handlers::change_path(t_client *client)
{
    t_request           *req;
    t_response          *res;
    t_location_configs  *d_configs;
    t_server_configs    *s_configs;
    std::string         path_after_change;
    std::string         current_dir;

    req = client->request;
    res = client->response;
    d_configs = res->dir_configs;
    s_configs = res->configs;
    path_after_change = "";
    current_dir = client->cwd;
    if (d_configs)
    {
        path_after_change = get_path_after_change(client, d_configs->root);
        current_dir = client->cwd;
        res->rootfilepath = path_after_change;
        res->filepath = current_dir + "/" + res->rootfilepath;
    }
    else
    {
        res->rootfilepath = s_configs->root + "/" + req->path;
        res->filepath = current_dir + "/" + res->rootfilepath;
    }
    return (true) ;
}

/***
 * takes a file a sets it to the root string
 * returns true if the s = (root + file) is exist and has the R_OK access
*/

bool    Handlers::set_path_for_file(std::string &basepath, std::string &root, std::string &file)
{
    std::string curr;
    std::string fullpath;

    curr = basepath;
    fullpath = curr + "/" + root + "/" + file;
    if (!access(fullpath.c_str(), R_OK))
    {
        root = root + "/" + file;
        return (true) ;
    }
    return (false) ;
}

/**
 * a function that handles the 400 code
 * 400 is for Bad Request
*/
bool    Handlers::handle_400(t_client *client)
{
    t_request *req = client->request;
    t_response *res = client->response;
    std::map<std::string, std::string>  *request_map = &req->request_map;
    std::map<int, std::string>          code_to_page;
    std::string error_page = "";
    std::string path = "";

    
    if (res->dir_configs)
        code_to_page = res->dir_configs->code_to_page;
    else
        code_to_page = res->configs->code_to_page;
    if ((req->method == "POST" && (!IN_MAP((*request_map), "content-type") || (!IN_MAP((*request_map), "transfer-encoding") && \
    !IN_MAP((*request_map), "content-length")))) || !is_request_uri_valid(req->path))
    {
        error_page = code_to_page[400];
        path = res->root;
        if (IN_MAP(code_to_page, 400) && set_path_for_file(client->cwd, path, error_page)) // error code is exist page provided in config file
        {
            res->filepath = client->cwd + "/" + path + "/" + error_page;
            res->filepath = get_cleanified_path(res->filepath);
            res->extension = get_extension(res->filepath);
            res->is_cgi = IS_CGI_EXTENSION(res->extension);
            if (res->is_cgi)
                res->cgi_path = res->configs->extension_cgi[res->extension];
            fill_response(client, 400, true);
            client->state = (req->method == "HEAD" ? SERVED : SERVING_GET);
            client->state = (client->state == SERVING_GET && res->is_cgi) ? SERVING_CGI : SERVING_GET;
            client->request->method = "GET";
        }
        else
        {
            res->filepath = "";
            res->rootfilepath = "";
            res->root = "";
            fill_response(client, 400, true);
            client->state = SERVED;
        }
        return true ;
    }
    return false ;
}

/**
 * a function that handles the 414 code
 * 414 is for Request URI too long
*/

bool Handlers::handle_414(t_client *client)
{
    t_request *req = client->request;
    t_response *res = client->response;
    std::map<int, std::string>          code_to_page;
    std::string error_page = "";
    std::string path = "";

    if (res->dir_configs)
        code_to_page = res->dir_configs->code_to_page;
    else
        code_to_page = res->configs->code_to_page;
    if (sz(req->path) >= MAX_REQUEST_URI_SIZE)
    {
        error_page = code_to_page[414];
        path = res->root;
        if (IN_MAP(code_to_page, 414) && set_path_for_file(client->cwd, path, error_page)) // error code is exist page provided in config file
        {
            res->filepath = client->cwd + "/" + path + "/" + error_page;
            res->filepath = get_cleanified_path(res->filepath);
            res->extension = get_extension(res->filepath);
            res->is_cgi = IS_CGI_EXTENSION(res->extension);
            if (res->is_cgi)
                res->cgi_path = res->configs->extension_cgi[res->extension];
            fill_response(client, 414, true);
            client->state = (req->method == "HEAD" ? SERVED : SERVING_GET);
            client->state = (client->state == SERVING_GET && res->is_cgi) ? SERVING_CGI : SERVING_GET;
            client->request->method = "GET";
        }
        else
        {
            res->filepath = "";
            res->rootfilepath = "";
            fill_response(client, 414, true);
            client->state = SERVED;
        }
        return true ;
    }
    return false ;
}

/**
 * a function that handles the 501 code
 * 501 is for Not Implemented mainly used if the client request a transfer-encoding different
 * than chunked which is what's our webserv is capable of
 * 
*/

bool    Handlers::handle_501(t_client *client)
{
    t_request *req = client->request;
    t_response *res = client->response;
    std::map<std::string, std::string>::iterator it = req->request_map.find("transfer-encoding");
    std::map<int, std::string> code_to_page;
    std::string error_page = "";
    std::string path = "";

    if (res->dir_configs)
        code_to_page = res->dir_configs->code_to_page;
    else
        code_to_page = res->configs->code_to_page;
    if (it != req->request_map.end())
    {
        if (it->second == "chunked")
            req->is_chunked = true;
        else
        {
            error_page = code_to_page[501];
            path = res->root;
            if (IN_MAP(code_to_page, 501) && set_path_for_file(client->cwd, path, error_page)) // error code is exist page provided in config file
            {
                res->filepath = client->cwd + "/" + path + "/" + error_page;
                res->filepath = get_cleanified_path(res->filepath);
                res->extension = get_extension(res->filepath);
                res->is_cgi = IS_CGI_EXTENSION(res->extension);
                if (res->is_cgi)
                    res->cgi_path = res->configs->extension_cgi[res->extension];
                fill_response(client, 501, true);
                client->state = (req->method == "HEAD" ? SERVED : SERVING_GET);
                client->state = (client->state == SERVING_GET && res->is_cgi) ? SERVING_CGI : SERVING_GET;
                client->request->method = "GET";
            }
            else
            {
                res->filepath = "";
                res->rootfilepath = "";
                fill_response(client, 501, true);
                client->state = SERVED;
            }
            return true ;
        }
    }
    return false ;
}

/**
 * a function that handles the 413 code
 * 413 is for Payload too Large
 * in term of our webserv it is used in case the client has sent us a large content-length
 * that is larger than the defined max_body_size
*/

bool Handlers::handle_413(t_client *client)
{
    t_request *req = client->request;
    t_response *res = client->response;
    t_server_configs *sconf = client->server->server_configs;
    std::map<std::string, std::string>  *request_map;
    std::map<int, std::string>          code_to_page;
    std::string                         error_page = "";
    std::string                         path = "";

    request_map = &req->request_map;
    if (res->dir_configs)
        code_to_page = res->dir_configs->code_to_page;
    else
        code_to_page = res->configs->code_to_page;
    if (IN_MAP((*request_map), "content-length"))
    {
        req->content_length = std::atoi(request_map->at("content-length").c_str());
        if((int)sconf->max_body_size < req->content_length)
        {
            error_page = code_to_page[413];
            path = res->root;
            if (IN_MAP(code_to_page, 413) && set_path_for_file(client->cwd, path, error_page)) // error code is exist page provided in config file
            {
                res->filepath = client->cwd + "/" + path + "/" + error_page;
                res->filepath = get_cleanified_path(res->filepath);
                res->extension = get_extension(res->filepath);
                res->is_cgi = IS_CGI_EXTENSION(res->extension);
            if (res->is_cgi)
                res->cgi_path = res->configs->extension_cgi[res->extension];
                fill_response(client, 413, true);
                client->state = (req->method == "HEAD" ? SERVED : SERVING_GET);
                client->state = (client->state == SERVING_GET && res->is_cgi) ? SERVING_CGI : SERVING_GET;
                client->request->method = "GET";
            }
            else
            {
                res->filepath = "";
                res->rootfilepath = "";
                fill_response(client, 413, true);
                client->state = SERVED;
            }
            return true ;
        }
    }
    return false;
}

/**
 * a function that handles the 405 code
 * 405 is for Method Not Allowed
*/

bool    Handlers::handle_405(t_client *client)
{
    t_request *req;
    t_response *res;
    t_location_configs  *l_configs;
    t_server_configs    *s_configs;
    std::map<int, std::string>          code_to_page;
    std::string                         error_page = "";
    std::string                         path = "";

    req = client->request;
    res = client->response;
    if (res->dir_configs)
        code_to_page = res->dir_configs->code_to_page;
    else
        code_to_page = res->configs->code_to_page;
    l_configs = res->dir_configs;
    s_configs = res->configs;
    std::vector<std::string>    allowed_methods = (l_configs) ? l_configs->allowed_methods : s_configs->allowed_methods;
    /*** Method is not allowed ****/
    if (std::find(allowed_methods.begin(), allowed_methods.end(), req->method) == allowed_methods.end())
    {
        error_page = code_to_page[405];
        path = res->root;
        if (IN_MAP(code_to_page, 405) && set_path_for_file(client->cwd, path, error_page)) // error code is exist page provided in config file
        {
            res->filepath = client->cwd + "/" + path + "/" + error_page;
            res->filepath = get_cleanified_path(res->filepath);
            res->extension = get_extension(res->filepath);
            res->is_cgi = IS_CGI_EXTENSION(res->extension);
            if (res->is_cgi)
                res->cgi_path = res->configs->extension_cgi[res->extension];
            fill_response(client, 405, true);
            client->state = (req->method == "HEAD") ? SERVED : SERVING_GET;
            client->state = (res->is_cgi && client->state == SERVING_GET) ? SERVING_CGI : SERVING_GET;
            req->method = "GET";
        }
        else
        {
            res->filepath = "";
            res->rootfilepath = "";
            fill_response(client, 405, true);
            client->state = SERVED;
        }
        return true ;
    }
    return (false);
}

/**
 * a function that handles the 301 code
 * 301 is for Moved Permanently which is redirection
 * if redirect is defined in our configfile in the path requested by the user 
*/

bool    Handlers::handle_301(t_client *client)
{
    t_response *res;

    res = client->response;
    if (!res->dir_configs)
        return false ;
    if (sz(res->dir_configs->redirection))
    {
        res->redirect_to = res->dir_configs->redirection;
        fill_response(client, 301, true);
        client->state = SERVED;
        return (true);
    }
    else if (IN_MAP(res->dir_configs->code_to_page, 301))
    {
        res->redirect_to = res->dir_configs->code_to_page[301];
        fill_response(client, 301, true);
        client->state = SERVED;
        return (true);
    }
    return (false) ;
}

/**
 * a function that handles the 404 code
 * 404 is for Not found
*/

bool    Handlers::handle_404(t_client *client)
{
    t_response          *res;
    t_location_configs  *d_configs;
    t_server_configs    *s_configs;
    std::string         root;
    std::vector<std::string> files_404;

    res = client->response;
    d_configs = res->dir_configs;
    s_configs = res->configs;
    files_404 = (d_configs) ? d_configs->pages_404 : s_configs->pages_404;
    root = res->root;
    if (set_file_path(client->cwd, root, files_404))
    {
        res->filepath = client->cwd + "/" + root;
        res->filepath = get_cleanified_path(res->filepath);
        res->extension = get_extension(res->filepath);
        res->is_cgi = IS_CGI_EXTENSION(res->extension); 
        if (res->is_cgi)
            res->cgi_path = res->configs->extension_cgi[res->extension];
        if (!access(res->filepath.c_str(), R_OK))
        {
            fill_response(client, 404, true);
            client->state = client->request->method == "HEAD" ? SERVED : SERVING_GET;
            client->state = (client->state == SERVING_GET && res->is_cgi) ? SERVING_CGI : SERVING_GET;
            client->request->method = "GET";
        }
        else
        {
            res->filepath = "";
            res->rootfilepath = "";
            fill_response(client, 403, true);
            client->state = SERVED;
        }
    }
    else
    {
        std::cout << "404 not found filepath => " << res->filepath << std::endl;
        res->rootfilepath = "";
        res->filepath = "";
        client->state = SERVED;
        fill_response(client, 404, true);
    }
    return (true) ;
}

/**
 * this function only called if everything is alright which means 200 code
 * and we are dealing with a directory (d at the end stands for directory)
*/
bool    Handlers::handle_200d(t_client *client)
{
    t_response          *res;
    t_location_configs  *d_configs;
    t_server_configs    *s_configs;
    std::vector<std::string> indexes;

    res = client->response;
    d_configs = res->dir_configs;
    s_configs = res->configs;
    indexes = (d_configs) ? d_configs->indexes : s_configs->indexes;
    if (d_configs)
        std::cout << YELLOW_BOLD << "WORKING WITH DIRECTORY CONFIGS" << std::endl;
    if (set_file_path(client->cwd, res->rootfilepath, indexes))
    {
        res->filepath = client->cwd + "/" + res->rootfilepath;
        res->filepath = get_cleanified_path(res->filepath);
        res->extension = get_extension(res->filepath);
        res->is_cgi = IS_CGI_EXTENSION(res->extension);
        if (res->is_cgi)
            res->cgi_path = res->configs->extension_cgi[res->extension];
        if (!access(res->filepath.c_str(), R_OK))
        {
            fill_response(client, 200, true);
            client->state = (res->is_cgi) ? SERVING_CGI : SERVING_GET;
            client->request->method = "GET";
        }
        else
        {
            res->filepath = "";
            res->rootfilepath = "";
            fill_response(client, 403, true);
            client->state = SERVED;
        }
    }
    else
    {
        std::cout << "CWD => " << client->cwd << std::endl;
        std::cout << "ROOT => " << res->rootfilepath << std::endl;

        std::string fpath = client->cwd + res->rootfilepath;
        DIR *d = opendir(fpath.c_str());
        if (d_configs && d_configs->directory_listing && d)
        {
            std::cout << "DIRECTORY LISTING" << std::endl;
            res->is_directory_listing = true ;
            fill_response(client, 200, true);
            client->state = SERVING_GET;
            client->request->method = "GET";
        }
        else
            handle_404(client);
        if (d)
            closedir(d);
    }
    return (true) ;
}

/**
 * this function only called if everything is alright which means 200 code
 * and we are dealing with a file (f at the end stands for file)
*/

bool Hanlders::check_is_cache_exists_for_cgi(t_client *client)
{
    t_response                              *res;
    t_request                               *req;
    std::string                             requested_uri;
    std::string                             cache_name;
    std::string                             cache_path;
    std::map<std::string, t_cache *>        *caches;

    res = client->response;
    req = client->request;
    caches = client->server->http_configs->caches;
    requested_uri = res->rootfilepath;
    if (!caches) // caches doesn't exist
        return false ;
    if (IN_MAP((*caches), res->filepath)) // is cache exist
    {
        cache_path = caches->at(res->filepath);
        res->filepath = cache_path;
        std::cout << GREEN_BOLD << "THE CACHE WAS FOUND FOR " << res->filepath << " -> " << cache_path << std::endl;
        return (true) ;
    }
    // cache_name = get_cache_name(requested_uri);
    // cache_path = std::string("caches/") + cache_name;
    return (false);
}

bool Handlers::handle_200f(t_client *client)
{
    t_response *res = client->response;
    t_request  *req = client->request;

    if (!access(res->filepath.c_str(), R_OK) && !is_directory_exist(client->cwd, res->rootfilepath)) // 200 ok
    {
        res->filepath = get_cleanified_path(res->filepath);
        res->extension = get_extension(res->filepath);
        res->is_cgi = IS_CGI_EXTENSION(req->extension);
        if (res->is_cgi)
        {
            if (check_is_cache_exists_for_cgi(client)) // sets the cache
            {
                res->filepath = cache_path;
                fill_response(client, 200, true);
            }
            res->cgi_path = res->configs->extension_cgi[req->extension];
        }
        }
        else
            fill_response(client, 200, true);
        client->state = (res->is_cgi) ? SERVING_CGI : SERVING_GET;
        client->request->method = "GET";
    }
    else if (!access(res->filepath.c_str(), F_OK) && !is_directory_exist(client->cwd, res->rootfilepath)) // 403 forbidden
    {
        res->filepath = "";
        res->rootfilepath = "";
        fill_response(client, 403, true);
        client->state = SERVED;
    }
    else // 404 not found
        handle_404(client);
    return (true) ;
}

/**
 * a function that handles the 200 code
 * 200 is for OK which means everything is good
 * in this function we have 2 possible cases
 * -----------------------------------------------
 * 1. we requested a directory
 *  - the index should be looked for and set
 *  - if the index was not found then 404 should be set
 *  - if not set then 404 not found without the page
 * 
 * 
 * 2. we requested a file
 *  - the file should be checked if exist before doing anything else
 *  if the file exists then it will get served
 * - if the file does not exist then we have to serve the 404 page if any exists
 *  otherwise, 404 without any page just the header will be served
*/

bool    Handlers::handle_200(t_client *client)
{
    change_path(client);
    if (client->request->is_file)
        handle_200f(client);
    else
        handle_200d(client);
    return (true);
}