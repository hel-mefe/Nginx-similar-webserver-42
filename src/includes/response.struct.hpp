#ifndef RESPONSE_STRUCT_HPP
# define RESPONSE_STRUCT_HPP

# include "socket.hpp"
# include "enums.hpp"
# include "globals.hpp"
# include "structs.hpp"
# include <sys/stat.h>

typedef struct response
{
    bool                                is_cgi;
    bool                                is_chunked;
    bool                                cgi_rn_found;
    bool                                cgi_running;
    bool                                is_directory_listing; // in case we have to list the directories inside
    bool                                is_first_time;
    int                                 del_files;
    int                                 cgi_pipe[2];
    int                                 fd;
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

    response() :is_cgi(false), fd(UNDEFINED), configs(nullptr), dir_configs(nullptr){
        cgi_rn_found = false;
        is_first_time = true;
        cgi_pipe[0] = UNDEFINED;
        cgi_pipe[1] = UNDEFINED;
        del_files = 0;
        buffer = new char[MAX_BUFFER_SIZE];
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
        // for (int i = 0; i < sz(s); i++)
        //     send(fd, &s[i], 1, 0);
        if (rn)
            send(fd, "\r\n", 2, 0);
    }

    void    write_response_in_socketfd(SOCKET fd)
    {
        std::map<std::string, std::string>::iterator it = response_map.begin();
        std::string ress = "HTTP/1.1 " + status_code + " " + status_line + "\r\n" ;

        while (it != response_map.end())
        {
            std::string first = it->first, second = it->second;
            ress += first + " : " + second + "\r\n";
            it++;
        }
        if (sz(this->filepath) && !this->is_directory_listing) // aslan filepath should be of sz == 0 in case directory_listing is on
        {
            // if (!is_chunked)
            //     ress += "Content-Length: " + std::to_string(get_file_size(this->filepath.c_str())) + "\r\n\r\n";
            // else
                ress += "Transfer-Encoding: Chunked\r\n\r\n";
        }
        else if (!this->is_directory_listing)
            ress += "\r\n";
        send(fd, ress.c_str(), sz(ress), 0);
        std::cout << ress << std::endl;
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