#ifndef RESPONSE_STRUCT_HPP
# define RESPONSE_STRUCT_HPP

# include "../socket.hpp"
# include "../enums.hpp"
# include "../../../includes/globals.hpp"
# include "../../../includes/structures.hpp"


typedef struct response
{
    std::map<std::string, std::string>  response_map;
    t_server_configs                    *configs;
    t_location_configs                  *dir_configs;
    std::string                         http_version;
    std::string                         status_code;
    std::string                         status_line;
    std::string                         directory_configs_path; // sz == 0 no d configs
    std::string                         filename;
    std::string                         filepath;
    std::string                         rootfilepath;
    std::string                         extension;
    std::string                         redirect_to;
    std::string                         cgi_path;
    bool                                is_cgi;
    bool                                cgi_rn_found;
    int                                 cgi_pipe[2];
    int                                 fd;
    int                                 code;

    response() : configs(nullptr), dir_configs(nullptr), is_cgi(false), fd(UNDEFINED){
        cgi_rn_found = false;
        cgi_pipe[0] = UNDEFINED;
        cgi_pipe[1] = UNDEFINED;
    }
    ~response(){}

    void    write_string(SOCKET fd, std::string s, bool rn)
    {
        // std::cout << YELLOW_BOLD << "write_string -> " << s << WHITE << std::endl;
        for (int i = 0; i < sz(s); i++)
            write(fd, &s[i], 1);
        if (rn)
            write(fd, "\r\n", 2);
    }

    void    write_response_in_socketfd(SOCKET fd)
    {
        std::map<std::string, std::string>::iterator it = response_map.begin();
        write_string(fd, http_version, false);
        write(fd, " ", 1);
        write_string(fd, status_code, false);
        write(fd, " ", 1);
        write_string(fd, status_line, true);
        while (it != response_map.end())
        {
            std::string first = it->first, second = it->second;
            write_string(fd, first, false);
            write(fd, ":", 1);
            write_string(fd, second, true);
            it++;
        }
        std::cout << "Response has been written in " << fd << std::endl;
        if (!this->is_cgi) // don't need this in case of cgi
            write(fd, "\r\n", 2);
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