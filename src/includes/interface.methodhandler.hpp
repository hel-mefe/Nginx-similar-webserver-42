#ifndef METHOD_HANDLER
# define METHOD_HANDLER

# include <iostream>

typedef struct client t_client;

class MethodHandler
{
    public:
        virtual void serve_client(t_client *client) = 0;
        virtual ~MethodHandler(){};
} ;

std::string intToString(int num);
char**      convert_cgi_env(t_client* client);
void        memory_freeder(char **env, char** args, int env_size);
void        fill_response(t_client *client, int code, std::string status_line, bool write_it);
void        fill_cgi_env(t_client* client);
void        serve_cgi(t_client* client, char** env, int args_size);
void        parse_cgi_output(t_client* client);
void        parse_chunked_body(t_client* client);
void        parse_non_chunked_body(t_client* client);
void        upload_file(t_client* client);
bool        create_file(t_client* client);
#endif