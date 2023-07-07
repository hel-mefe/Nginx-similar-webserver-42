#ifndef GET_HPP
# define GET_HPP

# include "interface.methodhandler.hpp"
# include "socket.hpp"
# include <dirent.h>

#define SOCKET int

class Get : public MethodHandler
{
    public:
        Get(){}
        void    serve_client(t_client *client);
        void    fill_response(t_client *client, int code, std::string status_line, bool write_it);
        char**  convert_cgi_env(t_client* client);
        void    fill_cgi_env(t_client* client);
        void    serve_cgi(t_client* client, char** env, int args_size);
        void    parse_cgi_output(t_client* client);
        ~Get(){}

        void    write_hex(SOCKET, int);
        void    write_chunk(SOCKET, unsigned char *, int);

        void    handle_static_file(t_client *client);
        void    handle_cgi(t_client *client);
<<<<<<< HEAD
        void    serve_cgi(t_client *client);
        void    handle_directory_listing(t_client *client);
=======
>>>>>>> atlas
        void    list_directories(t_client *client);

        char    **convert_env_map(std::map<std::string, std::string> &m);
        void    serve_directory_listing(t_client *client);
        void    write_schunk(SOCKET fd, std::string &s, int len);
        
} ;

std::string intToString(int num);
void    memory_freeder(char **env, char** args, int env_size);
void    remove_cgi_io(t_client* client);
#endif