#ifndef HTTP_HANDLER_UTILS_HPP
# define HTTP_HANDLER_UTILS_HPP

# include "socket.hpp"

typedef struct client t_client;

std::string                 get_cleanified_path(std::string s);
void                        alert(std::string s, const char *color);

bool                        is_file(std::string &path);
bool                        is_request_uri_valid(std::string &path);
bool                        is_path_valid(std::string &path);
bool                        set_file_path(std::string basepath, std::string &path, std::vector<std::string> &files);
bool                        is_directory_exist(std::string basepath, std::string &path);

std::string                 trim_string(std::string &s);
std::string                 get_upper_case(std::string s);
std::string                 get_lower_case(std::string s);
std::string                 get_extension(std::string &s);
std::string                 get_filename(std::string &s);
std::vector<std::string>    *split_first_line(std::string &s);

int     get_rn_endpos(unsigned char *buff, int buff_size); // -1 means not found
void    add_to_logs(t_client *client);

long long                           get_file_last_modified(const char *filename);
std::map<std::string, std::string>  get_cookies_queries_map(std::string &line, bool is_query);

#endif
