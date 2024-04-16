#ifndef RESPONSE_STRUCT_HPP
# define RESPONSE_STRUCT_HPP

# include "socket.hpp"
# include "enums.hpp"
# include "globals.hpp"
# include "structs.hpp"
# include <sys/stat.h>
# include "http_handler.utils.hpp"

#define IS_CGI_EXT(e) (e == ".php" || e == ".py" || e == ".pl")

typedef struct response
{
    bool                                is_cgi;
    bool                                is_chunked;
    bool                                cgi_rn_found;
    bool                                cgi_running;
    bool                                file_exist;
    bool                                is_directory_listing; // in case we have to list the directories inside
    bool                                is_first_time;
    int                                 del_files;
    int                                 cgi_pipe[2];
    int                                 fd;
    int                                 cache_fd;
    int                                 code;
    int                                 cgi_pid;
    char                                *buffer;
    t_server_configs                    *configs;
    t_location_configs                  *dir_configs;
    std::string                         http_version;
    std::string                         status_code;
    std::string                         status_line;
    std::string                         directory_configs_path; // sz == 0 no d configs
    std::string                         filename;
    std::string                         filepath;
    std::string                         root;
    std::string                         rootfilepath;
    std::string                         extension;
    std::string                         redirect_to;
    std::string                         cgi_path;
    std::list<std::pair<std::string, std::string> >      dir_link; // list => [(dir, link) pair]
    std::map<std::string, std::string>  response_map;
    std::map<std::string, std::string>  cgi_env;

    response() :is_cgi(false), fd(UNDEFINED), cache_fd(UNDEFINED), configs(nullptr), dir_configs(nullptr){
        cgi_rn_found = false;
        is_first_time = true;
        cgi_pipe[0] = UNDEFINED;
        cgi_pipe[1] = UNDEFINED;
        del_files = 0;
        buffer = new char[MAX_BUFFER_SIZE];
        file_exist = false;
    }
    ~response()
    {
        if (fd != UNDEFINED)
            close(fd);
        if (cgi_pipe[0] != UNDEFINED)
            close(cgi_pipe[0]);
        if (cgi_pipe[1] != UNDEFINED)
            close(cgi_pipe[1]);
        delete []buffer;
    }

    long    get_file_size(const char *filename)
    {
        struct stat file_status;

        if (stat(filename, &file_status) < 0)
            return -1;
        return (file_status.st_size);
    }

    void    write_string(SOCKET fd, std::string s, bool rn)
    {
        send(fd, s.c_str(), sz(s), 0);
        if (rn)
            send(fd, "\r\n", 2, 0);
    }

    bool    write_response_in_socketfd(SOCKET fd, bool finish_it)
    {
        std::map<std::string, std::string>::iterator it = response_map.begin();
        std::string ress = "HTTP/1.1 " + status_code + " " + status_line + "\r\n" ;

    if (this->is_directory_listing)
        response_map["connection"] = "closed" ;
        while (it != response_map.end())
        {
            std::string first = it->first, second = it->second;
            ress += first + " : " + second + "\r\n";
            it++;
        }
        if (finish_it)
            ress += "\r\n";
        if (send(fd, ress.c_str(), sz(ress), 0) == -1)
            return (false) ;
        return (true) ;
    }

    bool    add(std::string s1, std::string s2)
    {
        if (IN_MAP(response_map, s1))
            return (false);
        response_map.insert(std::make_pair(s1, s2));
        return (true);
    }

    bool remove(std::string s)
    {
        if (!IN_MAP(response_map, s))
            return (false);
        response_map.erase(s);
        return (true);
    }

    void    reset()
    {
        is_cgi = false;
        fd = UNDEFINED;
        cache_fd = UNDEFINED;
        configs = nullptr;
        dir_configs = nullptr;
        cgi_rn_found = false;
        is_first_time = true;
        cgi_pipe[0] = UNDEFINED;
        cgi_pipe[1] = UNDEFINED;
        del_files = 0;
        delete []buffer;
        buffer = new char[MAX_BUFFER_SIZE]();
        file_exist = false; 
        response_map.clear();
    }

    void    print_data()
    {
        std::cout << CYAN_BOLD << " **** PRINTING RESPONSE DATA **** " << WHITE_BOLD << std::endl;
        std::cout << "http version: " << http_version << std::endl;
        std::cout << "status code: " << status_code << std::endl;
        std::cout << "status line: " << status_line << std::endl;
        std::cout << "filename: " << filename << std::endl;
        std::cout << "redirect_to: " << redirect_to << std::endl;
        std::map<std::string, std::string>::iterator it = response_map.begin();
        while (it != response_map.end())
        {
            std::cout << it->first << ": " << it->second << std::endl;
            it++;
        }
        std::cout << CYAN_BOLD << " **** END PRINTING RESPONSE DATA ****" << std::endl;
    }

}   t_response ;

void    clarify_response(t_response *res);

#endif