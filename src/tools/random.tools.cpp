# include "../includes/http_handler.utils.hpp"

std::string intToString(int num) {
  std::stringstream ss;
  ss << num;
  return ss.str();
}

void    fill_response(t_client *client, int code, std::string status_line, bool write_it)
{
    t_request *req = client->request;
    t_response *res = client->response;
    std::string connection = req->get_param("connection"); // used for keep-alive

    std::cout << CYAN_BOLD << code << " : " << status_line << WHITE << std::endl;
    res->http_version = HTTP_VERSION;
    res->status_code = std::to_string(code);
    res->status_line = status_line ;
    if (sz(connection) && connection == "keep-alive") // type of connection
        res->add("connection", "keep-alive");
    if (write_it)
        res->write_response_in_socketfd(client->fd, !res->is_directory_listing);
}

std::string get_cleanified_path(std::string s)
{
    std::string res;
    bool flag = true;

    for (int i = 0; i < sz(s); i++)
    {
        if (s[i] != '/' || (s[i] == '/' && flag))
            res += s[i];
        flag = !(s[i] == '/');
    }
    return (res);
}

bool    is_file(std::string &path)
{
    int i = sz(path) - 1;

    for (; i >= 0 && path[i] != '/' && path[i] != '.'; i--);
    return (i >= 0 && path[i] == '.');
}

std::string trim_string(std::string &s)
{
    int a = 0, b = sz(s) - 1;
    while (a < b && (isspace(s[a]) || isspace(s[b])))
        a += (isspace(s[a])), b -= (isspace(s[a]));
    std::string res = s.substr(a, b - a + 1);
    return res;
}

// mainly used for methods
std::string get_upper_case(std::string s)
{
    std::string res = s;

    for (int i = 0; i < sz(s); i++)
    {
        if (isalpha(s[i]) && islower(s[i]))
            res[i] = toupper(s[i]);
    }
    return res ;
}

// mainly used for request parameters
std::string get_lower_case(std::string s)
{
    std::string res = s;

    for (int i = 0; i < sz(s); i++)
    {
        if (isalpha(s[i]) && isupper(s[i]))
            res[i] = tolower(s[i]);
    }
    return res;
}

std::string get_extension(std::string &s)
{
    std::string ext;
    int i = sz(s) - 1;

    for (; i >= 0 && s[i] != '.'; i--);
    if (i >= 0 && s[i] == '.')
        ext = s.substr(i, sz(s) - i);
    return ext;
}

std::string get_filename(std::string &s)
{
    size_t founds = s.find_last_of("/");
    size_t foundp = s.find_last_of(".");
    return(s.substr(founds + 1, foundp - (founds + 1)));
}

std::vector<std::string>    *split_first_line(std::string &s)
{
    std::vector<std::string>    *splitted = new std::vector<std::string>();
    int i = 0;
    for (; i < sz(s) && isspace(s[i]); i++);
    while (i < sz(s) && sz((*splitted)) != 3)
    {
        if (!isspace(s[i]))
        {
            int a = i;
            while(i < sz(s) && !isspace(s[i]))
                i++;
            std::string substring = s.substr(a, i - a);
            std::string res = trim_string(substring);
            splitted->push_back(res);
        }
        else
            i++;
    }
    return (splitted);
}

bool    is_request_uri_valid(std::string &path)
{
    for (int i = 0; i < sz(path); i++)
    {
        if (!isalnum(path[i]) && path[i] != '/' && path[i] != '.' && path[i] != '-' && path[i] != '_')
            return (false) ;
    }
    return (true) ;
}

bool    is_path_valid(std::string &path)
{
    return (!access(path.c_str(), F_OK));
}

bool    set_file_path(std::string basepath, std::string &path, std::vector<std::string> &files)
{
    std::string cwdpath;
 
    cwdpath = basepath;
    for (int i = 0; i < sz(files); i++)
    {
        std::string rpath;
        if (path[sz(path) - 1] == '/')
            rpath = path + files[i];//  + files[i];
        else
            rpath = path + "/" + files[i];
        std::string fullpath = cwdpath;
        if (fullpath[sz(fullpath) - 1] == '/')
        {
            if (rpath[0] == '/')
                fullpath += rpath.substr(1);
            else
                fullpath += rpath;
        }
        else
        {
            if (rpath[0] == '/')
                fullpath += rpath;
            else
                fullpath += "/" + rpath;
        }
        if (is_path_valid(fullpath))
        {
            path = rpath;
            return true ;
        }
    }
    return false ;
}

int     get_rn_endpos(unsigned char *buff, int buff_size) // -1 means not found
{
    char    rn[4];
    int     counter;

    rn[0] = '\r';
    rn[1] = '\n';
    rn[2] = '\r';
    rn[3] = '\n';
    counter = 0;
    for (int i = 0; buff[i] && i < buff_size; i++)
    {
        if (buff[i] == rn[counter])
            counter++;
        else
            counter = 0;
        if (counter == 4)
            return (i + 1);
    }
    return (-1);
}

bool    is_directory_exist(std::string basepath, std::string &path)
{
    std::string fullpath;
    DIR         *d;

    fullpath = basepath;
    fullpath += path;
    d = opendir(fullpath.c_str());
    return (d != NULL);    
}

/** logs function that adds all the logs to the etc/logs file **/
void    add_to_logs(t_client* client)
{
    std::string str = client->cwd + "/etc/logs";
    int fd = open(str.c_str(), O_CREAT | O_APPEND | O_WRONLY, 0777);
    if (fd < 0)
        return;
    str.clear();
    str += "[";
    str += std::to_string(time(NULL));
    str += "] ";
    str += "SERVER:";
    str += client->response->configs->server_name;
    str += " HTTP/1.1 ";
    str += client->request->method;
    str += " ";
    str += client->request->path;
    str += " | ";
    str += client->response->status_code;
    str += " ";
    str += client->response->status_line;
    str += "\n";
    write(fd, str.c_str(), str.size());
    close(fd);
}

long long get_file_last_modified(const char *filename)
{
    struct stat info;

    if (!stat(filename, &info))
        return (info.st_mtime) ;
    return (-1) ;
}

long long get_file_size(const char *filename)
{
    struct stat info;

    if (!stat(filename, &info))
        return (info.st_size) ;
    return (-1) ;

}

long long get_cache_folder_size(const char *foldername)
{
    DIR         *dir;
    std::string d_name;
    std::string filepath;
    std::string s_foldername;
    long long   res = 0;
    struct dirent *entry;

    dir = opendir(foldername);
    if (!dir)
    {
        std::cout << "0 returned -> " << strerror(errno) << std::endl;
        return (0) ;
    }
    s_foldername = foldername;
    while (1)
    {
        entry = readdir(dir);
        if (!entry)
            break ;
        d_name = entry->d_name; 
        if (d_name != "." && d_name != ".." && entry->d_type == DT_REG) // not special directory and a file
        {
            if (d_name.substr(0, 6) == "cache_") // cache_file always starts with "cache_"
            {
                filepath = s_foldername;
                if (sz(filepath) && filepath[sz(filepath) - 1] != '/')
                    filepath += "/" ;
                filepath += d_name; 
                res += get_file_size(filepath.c_str());
            }
        }

    }
    closedir(dir);
    return (res) ;
}

std::map<std::string, std::string>  get_cookies_queries_map(std::string &line, bool is_query)
{
    size_t      s_point;
    size_t      e_point;
    size_t      equal_char_pos;
    std::string part;
    std::string key;
    std::string value;
    std::map<std::string, std::string>  res_map;

    s_point = -1;
    e_point = UNDEFINED;
    while (e_point != line.size())
    {
        e_point = line.find(",", s_point + 1);
        e_point = (e_point == std::string::npos ? line.size() : e_point);
        part = line.substr(s_point + 1, e_point - (s_point + 1));
        equal_char_pos = part.find("=");
        key = part.substr(0, equal_char_pos);
        value = part.substr(equal_char_pos + 1);
        key = trim_string(key);
        value = trim_string(value);
        if (is_query || key != "ssid")
            res_map.insert(std::make_pair(key, value));
        s_point = e_point;
    }
    return (res_map);
}

void    throw_msg(std::string msg, bool is_exit, MSG_TYPE msg_type)
{
    if (msg_type == INFO)
        std::cout << CYAN_BOLD << "[WEBSERVER42]: " << msg << std::endl;
    else if (msg_type == WARNING)
        std::cout << YELLOW_BOLD << "[WEBSERVER42]: " << msg << std::endl;
    else if (msg_type == ERROR)
        std::cout << RED_BOLD << "[WEBSERVER42]: " << msg << std::endl;
    else if (msg_type == SUCCESS)
        std::cout << GREEN_BOLD << "[WEBSERVER42]: " << msg << std::endl;
    else if (msg_type == NORMAL)
        std::cout << WHITE_BOLD << "[WEBSERVER42]: " << msg << std::endl;
    std::cout << WHITE_BOLD ;
    if (is_exit)
        exit(127) ;
}

std::string get_cache_file_name(std::string filepath)
{
    std::string cache_name;
    std::string part;
    size_t      pos;
    size_t      prev_pos;

    pos = filepath.find("/");
    prev_pos = -1;
    while (pos != std::string::npos)
    {
        part = filepath.substr(prev_pos + 1, pos - prev_pos - 1);
        cache_name += part;
        prev_pos = pos;
        pos = filepath.find("/", pos + 1);
    }
    return (cache_name);
}

void    catch_leaks(const char *msg)
{
    std::string k;

    std::cout << "[LEAKS_DBUGGER]: " << msg << std::endl;
    while (1)
    {
        std::getline(std::cin, k);
        if (k == "1")
            break ;  
    }
}