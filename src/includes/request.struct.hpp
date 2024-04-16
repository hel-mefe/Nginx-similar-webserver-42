#ifndef REQUEST_STRUCT_HPP
# define REQUEST_STRUCT_HPP

# include "socket.hpp"
# include "enums.hpp"
# include "globals.hpp"

typedef struct request
{
    int                                 file;
    int                                 body_size;
    int                                 hex;
    int                                 content_length;
    bool                                first_time;
    bool                                is_chunked;
    bool                                endwr;
    bool                                data;
    bool                                is_file;
    bool                                is_not_implemented; // priority goes to this one
    bool                                is_bad_request;
    std::string                         method;
    std::string                         path;
    std::string                         cgi_in;
    std::string                         cgi_out;
    std::string                         http_version;
    std::string                         extension;
    std::string                         filename;
    std::map<std::string, std::string>  request_map;
    std::vector<std::string>            lines;
    std::string                         body;
    std::string                         cookies;
    std::string                         queries;

    request()
    {
        body_size = 0;
        first_time = true;
        is_chunked = false;
        data = false;
        endwr = false;
        is_bad_request = false ;
        is_not_implemented = false;
    }
    ~request()
    {
        remove(cgi_in.c_str());
        remove(cgi_out.c_str());
    }

    bool is_provided(std::string e)
    {
        return IN_MAP(request_map, e);
    }
    
    std::string get_param(std::string parameter)
    {
        if (IN_MAP(request_map, parameter))
            return request_map[parameter] ;
        return "" ;
    }

    void    print_data()
    {
        std::cout << PURPLE_BOLD << " **** PRINTING REQUEST DATA **** " << WHITE_BOLD << std::endl;
        std::cout << "METHOD: " << method << std::endl;
        std::cout << "PATH: " << path << std::endl;
        std::cout << "HTTP_VERSION: " << http_version << std::endl;
        std::cout << "EXTENSION: " << extension << std::endl;
        std::cout << "TYPE: " << (is_file ? "file" : "directory") << std::endl;
        if (is_file)
            std::cout << "FILENAME: " << filename << std::endl;
        for (std::map<std::string, std::string>::iterator it = request_map.begin(); it != request_map.end(); it++)
            std::cout << it->first << ": " << it->second << std::endl;
        std::cout << PURPLE_BOLD << " **** PRINTING REQUEST DATA **** " << WHITE << std::endl;
    }

    void convert_hex(std::string str)
    {
        char* endpr;
        hex = std::strtoul(str.c_str(), &endpr, 16);
    }
    
}   t_request;

#endif