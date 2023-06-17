#ifndef REQUEST_STRUCT_HPP
# define REQUEST_STRUCT_HPP

# include "../socket.hpp"
# include "../enums.hpp"
# include "../../../includes/globals.hpp"

typedef struct request
{
    std::ofstream                       file;
    int                                 body_size;
    int                                 hex;
    bool                                file_stat;
    bool                                encod_stat;
    bool                                endwr;
    bool                                data;
    std::string                         method;
    std::string                         path;
    std::string                         http_version;
    std::string                         extension;
    std::string                         filename;
    std::map<std::string, std::string>  request_map;
    std::vector<std::string>            lines;
    std::string                         body;
    bool                                is_file;

    request(){body_size = 0; file_stat = false; encod_stat = false; data = false; endwr = false;}
    ~request(){}

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