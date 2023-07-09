# include "../inc/tokenschecker.class.hpp"

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
    return ;
}

TokensChecker& TokensChecker::operator=(const TokensChecker &c)
{
    return *this ;
}

TokensChecker::~TokensChecker()
{
    return ;
}

bool TokensChecker::is_directory(std::string s)
{
    return (sz(s) && s[0] == '/');
}

bool TokensChecker::is_short_int(std::string s)
{
    int i = 0, num = 0;

    if (!is_int(s))
        return (false);
    num = std::stoi(s);
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
    return (s == "GET" || s == "POST" || s == "DELETE");
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
    std::string fullpath = getwd(NULL);

    fullpath += path;
    return (is_extension(extension) && !access(fullpath.c_str(), X_OK));
}

bool TokensChecker::is_code(std::string s)
{
    int code;

    code = std::atoi(s.c_str());
    return (code > 0 && code < 515);
}

bool TokensChecker::is_multiplexer(std::string s)
{
    return (s == "poll");
}