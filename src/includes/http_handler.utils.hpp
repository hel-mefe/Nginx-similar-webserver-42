#ifndef HTTP_HANDLER_UTILS_HPP
# define HTTP_HANDLER_UTILS_HPP

# include "socket.hpp"
# include "enums.hpp"

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

long long   get_cache_folder_size(const char *foldername);
void        throw_msg(std::string msg, bool is_exit, MSG_TYPE msg_type);

void        reset_logs(bool ask_first);
void        reset_caches(bool ask_first);
void        init_cli_tokens(HashMap<std::string, TOKEN> &cli_tokens);
t_cli       *parse_and_get_cli(int ac, char **av);
bool        is_project_structure_valid();
void        check_is_project_well_structured();
void        help();

long long   get_file_size(const char *filename);
std::string get_cache_file_name(std::string filepath);

void    catch_leaks(const char *msg);
bool    set_error_page(std::map<int, std::string> &code_to_page, std::string &fullpath, int code);

#endif
