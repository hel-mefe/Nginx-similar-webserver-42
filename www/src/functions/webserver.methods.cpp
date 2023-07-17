# include "../includes/webserv.class.hpp"
# include "../includes/globals.hpp"
# include <dirent.h>

/***
* the main webserv class that encapsulates all the work 
***/

Webserver::Webserver()
{
    parser = new ConfigFileParser();
    http_configs = new t_http_configs();
    servers = new std::vector<t_server *>();
    multiplexer = nullptr;
    codes = nullptr;
    mimes = nullptr;
    cli = nullptr;
}

Webserver::Webserver(std::string _config_file)
{
    parser = new ConfigFileParser();
    http_configs = new t_http_configs();
    servers = new std::vector<t_server *>();
    mimes = new HashMap<std::string, std::string>();
    codes = new HashMap<int, std::string>();
    config_file = _config_file;
    multiplexer = nullptr;
    cli = nullptr;
}

Webserver::Webserver(std::string _config_file, MultiplexerInterface *_multiplexer)
{
    parser = new ConfigFileParser();
    http_configs = new t_http_configs();
    servers = new std::vector<t_server *>();
    config_file = _config_file;
    multiplexer = _multiplexer;    
    mimes = new HashMap<std::string, std::string>();
    codes = new HashMap<int, std::string>();
    cli = nullptr;
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
    }
    return (warnings);
}

void    Webserver::run() // sockets of all servers will run here
{
    http_configs->cli = cli;
    if (sz(cli->multiplexer)) // multiplexer defined in cli
    {
        std::cout << "Multiplexer => " << cli->multiplexer << std::endl;
        http_configs->multiplexer = cli->multiplexer;
    }
    init_mimes();
    init_codes();
    set_multiplexer();
    multiplexer->set_configs(http_configs);
    multiplexer->set_servers(servers);
    multiplexer->set_mimes(mimes);
    multiplexer->set_codes(codes);
    multiplexer->multiplex();
}