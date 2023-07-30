# include "../includes/tokenschecker.class.hpp"

# define MAX_SHORT_INT 65536

/***
 * this class is mainly kept only for checking tokens if valid or not
 * the list of tokens is specified as follows
***/

bool is_digit(char c)
{
    return (c >= '0' && c <= '9');
}

TokensChecker::TokensChecker()
{
    return ;
}

TokensChecker::TokensChecker(const TokensChecker& c)
{
    (void)c;
    return ;
}

TokensChecker& TokensChecker::operator=(const TokensChecker &c)
{
    (void)c;
    return *this ;
}

TokensChecker::~TokensChecker()
{
    return ;
}

bool TokensChecker::is_directory(std::string s)
{
    return (!!sz(s));
    // return (sz(s) && s[0] == '/');
}

bool TokensChecker::is_short_int(std::string s)
{
    int num = 0;

    if (!is_int(s))
        return (false);
    num = std::atoi(s.c_str());
    return (num < MAX_SHORT_INT);
}

bool TokensChecker::is_int(std::string s)
{
    for (int i = 0; i < sz(s); i++)
    {
        if (!is_digit(s[i]))
            return (false);
    }
    return (true);
}

bool TokensChecker::is_method(std::string s)
{
    return (s == "GET" || s == "POST" || s == "DELETE" || s == "OPTIONS" || s == "HEAD" || s == "PUT" || s == "TRACE");
}

bool TokensChecker::is_on_off(std::string s)
{
    return (s == "on" || s == "off");
}

bool TokensChecker::is_connection(std::string s)
{
    return (s == "keep-alive" || s == "close");
}

bool TokensChecker::is_extension(std::string s)
{
    return (s == ".py" || s == ".php" || s == ".pl");
}

bool TokensChecker::is_cgi(std::string extension, std::string path)
{
    (void)path;
    return (is_extension(extension)); 
    // return (is_extension(extension) && !access(path.c_str(), X_OK));
}

bool TokensChecker::is_code(std::string s)
{
    int code;

    code = std::atoi(s.c_str());
    return (code > 0 && code < 515);
}

bool TokensChecker::is_multiplexer(std::string s)
{
    if ((MAIN_MULTIPLEXER == KQUEUE && s == "epoll") || (MAIN_MULTIPLEXER == EPOLL && s == "kqueue"))
    {
        std::cerr << "[Webserv42]: Unsupported multiplexing method provided in the config file" << std::endl;
        return (false) ;
    }
    return (s == "poll" || s == "kqueue" || s == "epoll" || s == "select");
}

bool TokensChecker::is_date(std::string s)
{
    int         i;
    std::string ext;

    i = 0;
    for (; i < sz(s) && isdigit(s[i]); i++);
    ext = s.substr(i);
    for (int j = 0; j < sz(ext); j++)
        ext[j] = tolower(ext[j]);
    return (ext == "d" || ext == "m" || ext == "s" || ext == "h");
}

bool TokensChecker::is_size(std::string s)
{
    int         i;
    std::string ext;

    i = 0;
    for (; i < sz(s) && isdigit(s[i]); i++);
    ext = s.substr(i);
    for (int j = 0; j < sz(ext); j++)
        ext[j] = tolower(ext[j]);
    return ((ext == "kb" || ext == "mb" || ext == "gb" || ext == "by"));
}