#ifndef HTTP_HANDLER_UTILS_HPP
# define HTTP_HANDLER_UTILS_HPP

# include "socket.hpp"


void                        alert(std::string s, const char *color);

bool                        is_file(std::string &path);
bool                        is_request_uri_valid(std::string &path);
bool                        is_path_valid(std::string &path);
bool                        set_file_path(std::string &path, std::vector<std::string> &files);
bool                        is_directory_exist(std::string &path);

std::string                 trim_string(std::string &s);
std::string                 get_upper_case(std::string s);
std::string                 get_lower_case(std::string s);
std::string                 get_extension(std::string &s);
std::string                 get_filename(std::string &s);
std::vector<std::string>    *split_first_line(std::string &s);

int     get_rn_endpos(unsigned char *buff, int buff_size); // -1 means not found

#endif
