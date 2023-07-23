# include "../includes/webserv.class.hpp"
# include "../includes/globals.hpp"
# include <dirent.h>
# include <istream>

/***
* the main webserv class that encapsulates all the work 
***/

Webserver::Webserver()
{
    parser = new ConfigFileParser();
    http_configs = new t_http_configs();
    http_configs->cwd = getwd(NULL);
    servers = new std::vector<t_server *>();
    multiplexer = nullptr;
    codes = nullptr;
    mimes = nullptr;
    caches = nullptr;
    cli = nullptr;
}

Webserver::Webserver(std::string _config_file)
{
    parser = new ConfigFileParser();
    http_configs = new t_http_configs();
    http_configs->cwd = getwd(NULL);
    servers = new std::vector<t_server *>();
    mimes = new HashMap<std::string, std::string>();
    codes = new HashMap<int, std::string>();
    config_file = _config_file;
    multiplexer = nullptr;
    caches = nullptr;
    cli = nullptr;
}

Webserver::Webserver(std::string _config_file, MultiplexerInterface *_multiplexer)
{
    parser = new ConfigFileParser();
    http_configs = new t_http_configs();
    http_configs->cwd = getwd(NULL);
    servers = new std::vector<t_server *>();
    config_file = _config_file;
    multiplexer = _multiplexer;    
    mimes = new HashMap<std::string, std::string>();
    codes = new HashMap<int, std::string>();
    cli = nullptr;
    caches = nullptr;
}

Webserver::Webserver(const Webserver& w)
{
    (void)w;
    return ;
}

Webserver& Webserver::operator=(const Webserver& w)
{
    (void)w;
    return *this;
}

Webserver::~Webserver()
{
    delete parser;
    delete http_configs;
    for (int i = 0; i < sz((*servers)); i++)
        delete servers->at(i);
    delete servers;
    delete mimes;
    delete codes;
    return ;
}

void    Webserver::print_global_http_data()
{
    std::cout << "**** GLOBAL HTTP CONFIGS ****" << std::endl;
    std::cout << "root: " << http_configs->root << std::endl;
    std::cout << "indexes: [ ";
    for (int i = 0; i < sz(http_configs->indexes); i++)
        std::cout << http_configs->indexes[i] << " ";
    std::cout << "]" << std::endl;
    std::cout << "allowed_methods: [ ";
    for (int i = 0; i < sz(http_configs->allowed_methods); i++)
        std::cout << http_configs->allowed_methods[i] << " ";
    std::cout << "]" << std::endl;
    std::cout << "client_max_body_size: " << http_configs->max_body_size << std::endl;
    std::cout << "client_max_request_timeout: " << http_configs->max_request_timeout << std::endl;
    std::cout << "cgi_max_request_timeout: " << http_configs->max_cgi_timeout << std::endl;
    std::cout << "keep_alive_max_timeout: " << http_configs->keep_alive_timeout << std::endl;
    std::cout << "multiplexer: " << http_configs->multiplexer << std::endl;
    std::cout << "cookies: " << ((http_configs->cookies) ? "on" : "off") << std::endl; 
    std::cout << "cwd: " << http_configs->cwd << std::endl;
}

void    Webserver::print_servers_data()
{
    t_server *s;

    for (int i = 0; i < sz((*servers)); i++)
    {
        s = servers->at(i);
        s->print_data();
    }
}

void    Webserver::print_all_data()
{
    print_global_http_data();
    print_servers_data();
}

bool    Webserver::parse_config_file() // call the parser and returns if the config file is valid or not
{
    if (!parser->parse_config_file(config_file, http_configs, servers))
        return (false);
    return (true);
}

void    Webserver::init_mimes()
{
    mimes->insert(std::make_pair(".aac", "audio/aac"));
    mimes->insert(std::make_pair(".css", "text/css"));
    mimes->insert(std::make_pair(".csv", "text/csv"));
    mimes->insert(std::make_pair(".doc", "application/msword"));
    mimes->insert(std::make_pair(".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"));
    mimes->insert(std::make_pair(".epub", "application/epub+zip"));
    mimes->insert(std::make_pair(".gif", "image/gif"));
    mimes->insert(std::make_pair(".htm", "text/html"));
    mimes->insert(std::make_pair(".html", "text/html"));
    mimes->insert(std::make_pair(".ico", "image/vnd.microsoft.icon"));
    mimes->insert(std::make_pair(".jpg", "image/jpeg"));
    mimes->insert(std::make_pair(".jpeg", "image/jpeg"));
    mimes->insert(std::make_pair(".js", "text/javascript"));
    mimes->insert(std::make_pair(".json", "application/json"));
    mimes->insert(std::make_pair(".jsonld", "application/ld+json"));
    mimes->insert(std::make_pair(".mp3", "audio/mpeg"));
    mimes->insert(std::make_pair(".mp4", "video/mp4"));
    mimes->insert(std::make_pair(".png", "image/png"));
    mimes->insert(std::make_pair(".pdf", "application/pdf"));
    mimes->insert(std::make_pair(".php", "application/x-httpd-php"));
    mimes->insert(std::make_pair(".ppt", "application/vnd.ms-powerpoint"));
    mimes->insert(std::make_pair(".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"));
    mimes->insert(std::make_pair(".rar", "application/vnd.rar"));
    mimes->insert(std::make_pair(".rtf", "application/rtf"));
    mimes->insert(std::make_pair(".sh", "application/x-sh"));
    mimes->insert(std::make_pair(".svg", "image/svg+xml"));
    mimes->insert(std::make_pair(".txt", "text/plain"));
    mimes->insert(std::make_pair(".weba", "audio/webm"));
    mimes->insert(std::make_pair(".webm", "video/webm"));
    mimes->insert(std::make_pair(".webp", "image/webp"));
    mimes->insert(std::make_pair(".xhtml", "application/xhtml+xml"));
    mimes->insert(std::make_pair(".xls", "application/vnd.ms-excel"));
    mimes->insert(std::make_pair(".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"));
    mimes->insert(std::make_pair("xml", "application/xml"));
    mimes->insert(std::make_pair(".zip", "application/zip"));
}

void    Webserver::init_codes()
{
    // Informational
    codes->insert(std::make_pair(100, "Continue"));
    codes->insert(std::make_pair(101, "Switching Protocols"));
    codes->insert(std::make_pair(102, "Processing"));
    codes->insert(std::make_pair(103, "Early Hints"));

    // Success
    codes->insert(std::make_pair(200, "OK"));
    codes->insert(std::make_pair(201, "Created"));
    codes->insert(std::make_pair(202, "Accepted"));
    codes->insert(std::make_pair(203, "Non-Authoritative Information"));
    codes->insert(std::make_pair(204, "No Content"));
    codes->insert(std::make_pair(205, "Reset Content"));
    codes->insert(std::make_pair(206, "Partial Content"));
    codes->insert(std::make_pair(207, "Multi-Status"));
    codes->insert(std::make_pair(208, "Already Reported"));
    codes->insert(std::make_pair(226, "IM Used"));

    // Redirection
    codes->insert(std::make_pair(300, "Multiple Choices"));
    codes->insert(std::make_pair(301, "Moved Permanently"));
    codes->insert(std::make_pair(302, "Found"));
    codes->insert(std::make_pair(303, "See Other"));


    // Client Errors
    codes->insert(std::make_pair(400, "Bad Request"));
    codes->insert(std::make_pair(401, "Unauthorized"));
    codes->insert(std::make_pair(402, "Payment Required"));
    codes->insert(std::make_pair(403, "Forbidden"));
    codes->insert(std::make_pair(404, "Not Found"));
    codes->insert(std::make_pair(405, "Method Not Allowed"));
    codes->insert(std::make_pair(406, "Not Acceptable"));
    codes->insert(std::make_pair(407, "Proxy Authentication Required"));
    codes->insert(std::make_pair(408, "Request Timeout"));
    codes->insert(std::make_pair(409, "Conflict"));
    codes->insert(std::make_pair(413, "Payload Too Large"));
    codes->insert(std::make_pair(414, "URI Too Long"));
    codes->insert(std::make_pair(415, "Unsupported Media Type"));
    codes->insert(std::make_pair(416, "Range Not Satisfiable"));
    codes->insert(std::make_pair(417, "Expectation Failed"));

    // Server Internal Errors
    codes->insert(std::make_pair(500, "Internal Server Error"));
    codes->insert(std::make_pair(501, "Not Implemented"));
    codes->insert(std::make_pair(502, "Bad Gateway"));
    codes->insert(std::make_pair(503, "Service Unavailable"));
    codes->insert(std::make_pair(504, "Gateway Timeout"));
    codes->insert(std::make_pair(505, "HTTP Version Not Supported"));
    codes->insert(std::make_pair(506, "Variant Also Negotiates"));
    codes->insert(std::make_pair(507, "Insufficient Storage"));
    codes->insert(std::make_pair(508, "Loop Detected"));
    codes->insert(std::make_pair(510, "Not Extended"));
    codes->insert(std::make_pair(511, "Network Authentication Required"));
}

void    Webserver::set_multiplexer()
{
    std::string s_mult = http_configs->multiplexer;

    if (!sz(s_mult) || s_mult == "kqueue")
        this->multiplexer = new Kqueue();
    if (!sz(s_mult) || s_mult == "poll")
        this->multiplexer = new Poll();
    // else if (s_mult == "epoll")
    //     this->multiplexer = new Epoll();
}

void    Webserver::set_cli(t_cli *_cli)
{
    this->cli = _cli;
}

void    Webserver::create_redirection_graph(t_server *server, HashMap<std::string, std::string> &graph)
{
    HashMap<std::string, t_location_configs *> *dir_configs;

    dir_configs = server->dir_configs;
    for (HashMap<std::string, t_location_configs *>::iterator it = dir_configs->begin(); it != dir_configs->end(); it++)
    {
        std::string location = it->first;
        t_location_configs *confs = it->second;
        std::string redirection = confs->redirection;

        redirection = (redirection[sz(redirection) - 1] != '/') ? redirection + "/": redirection;

        if (sz(redirection)) // there is redirection
            graph[location] = redirection;
    }
}

bool    Webserver::is_root_cycled(HashMap<std::string, std::string> &graph, std::string start)
{
    HashSet<std::string>    keep;

    while (1)
    {
        if (!IN_MAP(graph, start))
            return false;
        if (IN_MAP(keep, start))
            return (true);
        keep.insert(start);
        start = graph[start];        
    }
    return (false) ;
}

bool    Webserver::is_cycled(HashMap<std::string, std::string> &graph)
{
    for (HashMap<std::string, std::string>::iterator it = graph.begin(); it != graph.end(); it++)
    {
        std::string start = it->first;
        
        if (is_root_cycled(graph, start))
            return (true) ;
    }
    return (false) ;
}

void    Webserver::set_redirection_loop_warning(t_server *server, std::vector<std::string> &wrs)
{
    HashMap<std::string, std::string>   graph;
    std::string                         server_name;

    server_name = server->server_configs->server_name;
    create_redirection_graph(server, graph);
    if (is_cycled(graph))
    {
        std::string warning = server_name + " has an infinite redirection loop" ;
        wrs.push_back(warning);
    }
}

void    Webserver::set_invalid_root_warning(t_server *server, std::vector<std::string> &wrs)
{
    std::string root;;
    std::string name;
    DIR         *d;

    name = server->server_configs->server_name;
    root = server->server_configs->root;
    if (sz(root)) // if root is provided
    {
        d = opendir(root.c_str());
        if (!d)
        {
            std::string s = name + " has an invalid root directory [ " + root + " ]";
            wrs.push_back(s);
        }
    }
}

void    Webserver::set_redefined_location_warning(t_server *server, std::vector<std::string> &wrs)
{
    std::string             root;
    std::string             name;
    HashSet<std::string>                        keep;
    HashMap<std::string, t_location_configs *>  *dir_configs;

    dir_configs = server->dir_configs;
    root = server->server_configs->root;
    name = server->server_configs->server_name;
    for (HashMap<std::string, t_location_configs *>::iterator it = dir_configs->begin(); it != dir_configs->end(); it++)
    {
        std::string location = it->first;

        if (IN_MAP(keep, location))
        {
            std::string s = name + " has a redefined location [ " + location + " ]";

            wrs.push_back(s);
        }
        else
            keep.insert(location);
    }
}

void    Webserver::set_cgi_warning(t_server *server, std::vector<std::string> &wrs)
{
    std::string                         cgi_path;
    std::string                         name;
    HashMap<std::string, std::string>   ext_path;

    name = server->server_configs->server_name;
    ext_path = server->server_configs->extension_cgi;
    for (HashMap<std::string, std::string>::iterator it = ext_path.begin(); it != ext_path.end(); it++)
    {
        std::string path = it->second;
        std::string s;

        if (!access(path.c_str(), R_OK))
        {
            if (access(path.c_str(), X_OK))
                s = name + " has a cgi path that does not have the x permission [ " + path + " ]";
        }
        else
            s = name + " has a cgi path that does not exist [ " + path + " ]";
        if (sz(s))
            wrs.push_back(s);
    }
}

// bool    Webserver::is_cache_valid()
// {
//     int fd = open("caches/cacherc", O_RDWR | O_CREAT | O_APPEND, 0777);
//     char    buff[MAX_BUFFER_SIZE];

//     if (fd == -1)
//     {
//         std::cerr << "[Webserver42]: internal server file descriptor error, open failed!" << std::endl;
//         exit(1);
//     }
//     bzero(buff, MAX_BUFFER_SIZE);
//     int bts = -1;
//     while (bts)
//     {
//         int bts = read(fd, buff, MAX_BUFFER_SIZE);
        
//     }
// }

// void    Webserver::set_cache_warning(t_server *server, std::vector<std::string> &warnings)
// {
//     if (server->http_configs->cacherc_fd == UNDEFINED)
//         return ;
//     std::string s;

//     if (is_cache_valid())
//     {
//         s = "the cacherc is not valid " ;
//         warnings.push_back(s);
//     }
// }

std::vector<std::string>    Webserver::generate_all_warnings()
{
    std::vector<std::string>    warnings;

    // warnings will be generated for each server
    for (int i = 0; i < sz((*servers)); i++)
    {
        set_redirection_loop_warning(servers->at(i), warnings);
        set_invalid_root_warning(servers->at(i), warnings);
        set_redefined_location_warning(servers->at(i), warnings);
        set_cgi_warning(servers->at(i), warnings);
        // set_cache_warning(servers->at(i), warnings);
    }
    return (warnings);
}

/***
 * 
 * each line of the cache is as follows:
 *  file_requested_absolute_path | file_to_serve_absolute_path | queries (key=value,ke2=value2 ... keyn=valuen) | cookies (key=val, key2=val2, key3=val3 ... etc) | date_created_unix_timetamp
*/

std::string get_cache_requested_file(std::string &line)
{
    size_t      pipe_pos;
    std::string requested_file;

    pipe_pos = line.find_first_of("|");
    requested_file = line.substr(0, pipe_pos);
    requested_file = trim_string(requested_file);
    return (requested_file);
}

std::string get_cache_served_file(std::string &line)
{
    size_t      pipe_pos1;
    size_t      pipe_pos2;
    std::string served_file;

    pipe_pos1 = line.find("|");
    pipe_pos2 = line.find("|", pipe_pos1 + 1);
    served_file = line.substr(pipe_pos1 + 1, pipe_pos2 - (pipe_pos1 + 1));
    served_file = trim_string(served_file);
    return (served_file);
}

std::string get_cache_queries(std::string &line)
{
    size_t      pipe_pos1;
    size_t      pipe_pos2;
    std::string queries;

    pipe_pos1 = line.find("|");
    pipe_pos1 = line.find("|", pipe_pos1 + 1);
    pipe_pos2 = line.find("|", pipe_pos1 + 1);
    queries = line.substr(pipe_pos1 + 1, pipe_pos2 - (pipe_pos1 + 1));
    queries = trim_string(queries);
    return (queries);

}

std::string get_cache_cookies(std::string &line)
{
    size_t      pipe_pos1;
    size_t      pipe_pos2;
    std::string cookies;

    pipe_pos1 = line.find("|");
    pipe_pos1 = line.find("|", pipe_pos1 + 1);
    pipe_pos1 = line.find("|", pipe_pos1 + 1);
    pipe_pos2 = line.find("|", pipe_pos1 + 1);
    cookies = line.substr(pipe_pos1 + 1, pipe_pos2 - (pipe_pos1 + 1));
    cookies = trim_string(cookies);
    return (cookies);

}

std::string get_cache_date(std::string &line)
{
    size_t      pipe_pos1;
    size_t      pipe_pos2;
    std::string date_created;

    pipe_pos1 = line.find("|");
    pipe_pos1 = line.find("|", pipe_pos1 + 1);
    pipe_pos1 = line.find("|", pipe_pos1 + 1);
    pipe_pos2 = line.find("|", pipe_pos1 + 1);
    date_created = line.substr(pipe_pos2 + 1);
    date_created = trim_string(date_created);
   return (date_created);
}
/***
 * Cache will be in this format:
 *    file_requested_abs_path | file_serve_abs_path | qkey1=val1, qkey2=val2 | ckey1=cval1, ckey2=cval2 | unix_timestamp_creation_date
*/

void    Webserver::set_cache_data(t_cache *c, std::string &line)
{
    c->rq_file = get_cache_requested_file(line);
    c->s_file = get_cache_served_file(line);
    c->queries = get_cache_queries(line);
    c->cookies = get_cache_cookies(line);
    c->date_created = get_cache_date(line);
    c->t_created = std::atol(c->date_created.c_str());
    c->t_rq_last_modified = get_file_last_modified(c->rq_file.c_str());
    c->t_s_last_modified = get_file_last_modified(c->s_file.c_str());
    c->queries_map = get_cookies_queries_map(c->queries, true);
    c->cookies_map = get_cookies_queries_map(c->cookies, false);
    c->is_valid = IS_CACHE_VALID(c);

    std::cout << "REQUESTED_FILE = " << c->rq_file << std::endl;
    std::cout << "SERVED FILE = " << c->s_file << std::endl;
    std::cout << "QUERIES = " << c->queries << std::endl;
    std::cout << "COOKIES = " << c->cookies << std::endl;
    std::cout << "DATE_CREATED = " << c->date_created << std::endl;
    std::cout << "REQUEST_LAST_MODIFIED = " << c->t_rq_last_modified << std::endl;
    std::cout << "SERVED_LAST_MODIFIED = " << c->t_s_last_modified << std::endl;
    std::cout << "*** PRINTING QUERIES ***" << std::endl;
    for (std::map<std::string, std::string>::iterator it = c->queries_map.begin(); it != c->queries_map.end(); it++)
        std::cout << it->first << " -> " << it->second << std::endl;
    std::cout << "*** PRINTING COOKIES ***" << std::endl;
    for (std::map<std::string, std::string>::iterator it = c->cookies_map.begin(); it != c->cookies_map.end(); it++)
        std::cout << it->first << " -> " << it->second << std::endl;
    std::cout << "IS_VALID = " << (c->is_valid ? "YES" : "NO") << std::endl;
}

bool    Webserver::parse_cache_line(std::string &line)
{
    t_cache *c;

    c = new t_cache();
    set_cache_data(c, line);
    if (c->is_valid)
        (*caches)[c->rq_file] = c;
    else
    {
        delete c;
        return (false);
    }
    return (true) ;
    // if (!access(requested_file.c_str(), R_OK) && !access(served_file.c_str(), R_OK))
    // {
    //     long long dc_time = std::atoll(date_created.c_str());
    //     long long last_modified = get_file_last_modified(requested_file.c_str());
    //     std::cout << dc_time << " - " << last_modified << (dc_time > last_modified ? " YES" : " NO") << std::endl;
    //     if (last_modified > dc_time) // then the file is valid
    //     {
    //         std::cout << CYAN_BOLD << requested_file << " -> " << served_file << " is valid!" << std::endl;
    //         (*caches)[requested_file] = served_file;
    //     }
    // }


}

void    Webserver::parse_cache(std::vector<std::string> &warnings)
{
    std::string line;
    std::ifstream st;
    int           fd;
    std::string    warn_s;
    bool          is_warning_set = false;

    if (access("caches/cacherc", R_OK))
    {
        fd = open("caches/cacherc", O_CREAT, 0777);
        if (fd != -1)
            close(fd);
    }
    st.open("caches/cacherc");
    while (st.good())
    {
        getline(st, line);
        if (sz(line))
        {
            if (!parse_cache_line(line) && !is_warning_set)
            {
                warn_s = "Some cached data is corrupted inside the [caches/cachrc] file, consider resetting the cache to avoid this warning";
                warnings.push_back(warn_s);
                is_warning_set = true;
            }
        }
    }
    st.close();
}

void    Webserver::reset_cache()
{
    std::string     path;
    std::string     prefix;
    std::string     entry_name;
    DIR             *dir;
    struct dirent   *entry;

    path = "/caches";
    dir = opendir(path.c_str());
    if (!dir)
        return ;
    while (1)
    {
        entry = readdir(dir);
        if (!entry)
            break ;
        entry_name = entry->d_name;
        prefix = entry_name.substr(0, 6);
        if ((entry_name == "." || entry_name == "..") && entry->d_type == DT_REG && prefix == "cache_")
        {
            path += "/" + entry_name;
            unlink(path.c_str());
        }
    }    
    closedir(dir);
}

/**
 * cachetime file will be templated as follows:
 *    time_created:     [number]
 *    valid_until:      [number]
 *    size:             [number]
 * 
 * 
*/

bool    Webserver::set_cache_time_data(t_http_configs *http_configs, std::vector<std::string> &warnings)
{
    std::ifstream st;
    int           fd;
    std::string   line;
    std::string   part1;
    std::string   part2;
    ll            number;
    size_t        spoint;
    bool          tc_found = false, vu_found = false, sz_found = false, tv_found = false;
    ll            time_created = UNDEFINED, valid_until = UNDEFINED, cache_size = UNDEFINED, time_valid = UNDEFINED;

    if (access("caches/cachetm", R_OK))
    {
        fd = open("caches/cachetm", O_CREAT, 0777);
        if (fd != -1)
            close(fd);
    }
    st.open("caches/cachetm");
    while (st.good())
    {
        std::getline(st, line);
        spoint = line.find(":");
        spoint = (spoint == std::string::npos) ? 0 : spoint;
        line = trim_string(line);
        if (sz(line) && line[0] != '#') // # is kept for comments
        {
            part1 = line.substr(0, spoint);
            part2 = line.substr(spoint + 1);
            part1 = trim_string(part1);
            part2 = trim_string(part2);
            number = std::atoll(part2.c_str());
            if (part1 == "time_created")
                tc_found = true, time_created = number;
            else if (part1 == "time_expired")
                vu_found = true, valid_until = number;
            else if (part1 == "time_valid")
                tv_found = true, time_valid = number;
            else if (part1 == "size")
                sz_found = true, cache_size = number;
            else if (sz(part1) || sz(part2))
            {
                std::string s = "Cache time file [cachetm] is corrupted, consider removing it or clearing it to avoid this warning";
                std::cout << s << std::endl;
                warnings.push_back(s);
                SET_CACHE_OFF(http_configs);
                return (false);
            }
        }
    }
    /** all the parameters should be exist in the file **/
    if ((tc_found || vu_found || tv_found || sz_found) && (!tc_found || !vu_found || !tv_found || !sz_found))
    {
        std::string s = "Cache time file [cachetm] is corrupted, consider removing it or clearing it to avoid this warning";
        std::cout << s << std::endl;
        warnings.push_back(s);
        return (false);
    }
    std::cout << GREEN_BOLD << "time_created -> " << time_created << std::endl;
    std::cout << "time_expired -> " << valid_until << std::endl;
    std::cout << "time_valid -> " << valid_until << std::endl;
    std::cout << "cache_size -> " << cache_size << std::endl;
    /** start parsing the data **/

    /** in case the we'll work with the proxy_cache **/
    if (http_configs->proxy_cache || http_configs->proxy_cache_register)
    {
        if (IS_CACHE_EXPIRED(valid_until))
            reset_cache() ;
        else if (IS_CACHE_PASSED_SIZE(cache_size))
        {
            std::string s = "Cache has been exceeded the provided size, please consider resetting it [./webserv --reset-cache]";
            warnings.push_back(s);
            http_configs->proxy_cache_register = false;
        }
    }
    return (true);
}

void    Webserver::run() // sockets of all servers will run here
{
    http_configs->cli = cli;
    std::vector<std::string>    warnings;

    std::cout << "NUMBER OF SERVERS => " << sz((*servers)) << std::endl;
    for (int i = 0; i < sz((*servers)); i++)
        servers->at(i)->http_configs = http_configs;

    /*** CLI always has the priority over config file ***/
    if (sz(cli->multiplexer)) // multiplexer defined in cli
        http_configs->multiplexer = cli->multiplexer;
    if (cli->is_logs_activated)
        http_configs->proxy_logs_register = cli->is_logs_activated;
    if (cli->is_cache_register_activated)
        http_configs->proxy_cache_register = true ;
    if (cli->is_cache_activated)
        http_configs->proxy_cache = true;

    /*** setting all the cache data ***/
    warnings = generate_all_warnings();
    set_cache_time_data(http_configs, warnings);
    // exit(1);
    if (http_configs->proxy_cache || http_configs->proxy_cache_register)
    {
        this->caches = new std::map<std::string, t_cache*>();
        parse_cache(warnings);
        std::cout << GREEN_BOLD << "The cache is parsed" << std::endl;
    }

    init_mimes();
    init_codes();
    set_multiplexer();

    /*** setting the necessary data for the multiplexer ***/
    multiplexer->set_configs(http_configs);
    multiplexer->set_servers(servers);
    multiplexer->set_mimes(mimes);
    multiplexer->set_codes(codes);
    if (http_configs->proxy_cache || http_configs->proxy_cache_register)
        http_configs->caches = caches;
    multiplexer->multiplex();
}