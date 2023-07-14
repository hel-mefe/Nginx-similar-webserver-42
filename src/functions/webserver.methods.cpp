# include "../includes/webserv.class.hpp"
# include "../includes/globals.hpp"

/***
* the main webserv class that encapsulates all the work 
***/

Webserver::Webserver()
{
    parser = new ConfigFileParser();
    http_configs = new t_http_configs();
    servers = new std::vector<t_server *>();
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
        this->multiplexer = new Kqueue() ;
    else if (s_mult == "poll")
        this->multiplexer = new Poll() ;
}

void    Webserver::run() // sockets of all servers will run here
{
    init_mimes();
    init_codes();
    set_multiplexer();
    multiplexer->set_configs(http_configs);
    multiplexer->set_servers(servers);
    multiplexer->set_mimes(mimes);
    multiplexer->set_codes(codes);
    multiplexer->multiplex();
}