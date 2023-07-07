#ifndef RESPONSE_STRUCT_HPP
# define RESPONSE_STRUCT_HPP

# include "socket.hpp"
# include "enums.hpp"
# include "globals.hpp"
# include "structs.hpp"


typedef struct response
{
    bool                                is_cgi;
    bool                                cgi_rn_found;
    bool                                cgi_running;
    bool                                is_directory_listing; // in case we have to list the directories inside
    bool                                is_first_time;
    int                                 cgi_pipe[2];
    int                                 fd;
    int                                 code;
    int                                 cgi_pid;
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
    std::list<std::pair<std::string, std::string>>      dir_link; // list => [(dir, link) pair]
    std::map<std::string, std::string>  response_map;
    std::map<std::string, std::string>  cgi_env;

    response() : configs(nullptr), dir_configs(nullptr), is_cgi(false), fd(UNDEFINED){
        cgi_rn_found = false;
        is_first_time = true;
        cgi_pipe[0] = UNDEFINED;
        cgi_pipe[1] = UNDEFINED;
    }
    ~response()
    {
        if (fd != UNDEFINED)
            close(fd);
        if (cgi_pipe[0] != UNDEFINED)
            close(cgi_pipe[0]);
        if (cgi_pipe[1] != UNDEFINED)
            close(cgi_pipe[1]);
    }

    void    write_string(SOCKET fd, std::string s, bool rn)
    {
        for (int i = 0; i < sz(s); i++)
            send(fd, &s[i], 1, 0);
        if (rn)
            send(fd, "\r\n", 2, 0);
    }

    void    write_response_in_socketfd(SOCKET fd, bool terminate)
    {
        std::map<std::string, std::string>::iterator it = response_map.begin();
        std::cout << GREEN_BOLD << " ********* HEADER SENT ********" << std::endl;
        write_string(fd, http_version, false);
        send(fd, " ", 1, 0);
        write_string(fd, status_code, false);
        send(fd, " ", 1, 0);
        write_string(fd, status_line, true);
        std::cout << http_version << " " << status_code << " " << status_line << "\\r\\n" << std::endl;
        while (it != response_map.end())
        {
            std::string first = it->first, second = it->second;
            write_string(fd, first, false);
            send(fd, ":", 1, 0);
            write_string(fd, second, true);
            std::cout << first << " : " << second << " \\r\\n" << std::endl;
            it++;
        }
        if (terminate)
        {
            send(fd, "\r\n", 2, 0);
            std::cout << "\\r\\n" << std::endl;
        }
        std::cout << WHITE_BOLD << "Response has been written in " << fd << std::endl;
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

#endif