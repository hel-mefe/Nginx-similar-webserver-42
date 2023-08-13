#ifndef TRACE_HPP
# define TRACE_HPP

# include "interface.methodhandler.hpp"
# include "socket.hpp"

class Trace : public MethodHandler
{
    public:
        Trace();
        ~Trace();
        std::vector<std::string> getForbiddenHeades(void);
        std::string getServerName(t_client *client);
        std::string getServerVersion(t_client *client);
        std::string setBody(t_client *client, std::vector<std::string> forbiddenHeaders);
        std::string getBodySize(std::string body);
        char* setDateHeader(char* dateStr, int size);
        void    serve_client(t_client *client);
} ;

#endif