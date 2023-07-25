# include "../includes/http_handler.utils.hpp"

std::string intToString(int num) {
  std::stringstream ss;
  ss << num;
  return ss.str();
}

void    memory_freeder(char **env, char** args, int env_size)
{
    for(int i = 0; i < env_size; i++)
    {
        if (env[i])    
            free(env[i]);
    }
    free(args[0]);
    free(args[1]);
    free(args);
    free(env);
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
        res->write_response_in_socketfd(client->fd);
}

char** convert_cgi_env(t_client* client)
{
    t_response* res = client->response;
    int env_size = res->cgi_env.size();
    char** args = (char**) malloc((env_size + 1) * sizeof(char*));
    std::cout << CYAN_BOLD << "... HANDLING CGI ..." << WHITE << std::endl;
    args[env_size] = NULL;
    std::map<std::string, std::string>::iterator it = res->cgi_env.begin();
    int i = 0;
    for(; it != res->cgi_env.end(); it++)
    {
        std::string arg = it->first + it->second;
        args[i] = strdup(arg.c_str());
        std::cout << args[i] << std::endl;
        i++;
    }
    return args;
}

void    fill_cgi_env(t_client* client)
{
    client->response->cgi_env["REQUEST_METHOD="] = client->request->method;
    client->response->cgi_env["REDIRECT_STATUS="] = "200";
    client->response->cgi_env["CONTENT_LENGTH="] = intToString(client->request->body_size);
    client->response->cgi_env["SCRIPT_FILENAME="] = client->response->filepath;
    client->response->cgi_env["PATH_INFO="] = client->response->filepath;
    if (client->request->method == "POST")
        client->response->cgi_env["CONTENT_TYPE="] = client->request->request_map.at("Content-Type");
    if (!client->request->cookies.empty())
        client->response->cgi_env["HTTP_COOKIE="] = client->request->cookies;
}

void    serve_cgi(t_client* client, char** env, int args_size)
{
    std::cout << "CGI_PATH=" << client->response->cgi_path << std::endl;
    char** args = (char **) malloc (3 * sizeof(char *));
    args[0] = strdup(client->response->cgi_path.c_str());
    args[1] = strdup(client->response->filepath.c_str());
    args[2] = NULL;
    for (int i = 0; i < INT_MAX; i++)
    {
        std::string file_path = "/tmp/cgi_out";
        if (i)
            file_path.append(intToString(i));
        if (access(file_path.c_str(), F_OK))
        {
            client->request->cgi_out = file_path;
            break;
        }
    }
    client->response->cgi_pid = fork();
    if (client->response->cgi_pid < 0)
    {
        std::cerr << RED_BOLD << "error: creating cgi process is failed!" << WHITE << std::endl;
        fill_response(client, 501, "Internal Server Error", true);
        client->state = SERVED;
        return;
    }
    if (!client->response->cgi_pid)
    {
        int cgi_io[2];
        if (client->request->method == "POST")
        {
            cgi_io[0] = open(client->request->cgi_in.c_str(), O_RDONLY);
            if (cgi_io[0] > 0)
                dup2(cgi_io[0], 0);
            else
                exit(42);
        }
        cgi_io[1] = open(client->request->cgi_out.c_str(), O_CREAT | O_APPEND | O_RDWR, 0777);
        if (cgi_io[1] > 0)
            dup2(cgi_io[1], 1);
        else
            exit(42);
        if (execve(args[0], args, env))
            exit(42);
    }
    memory_freeder(env, args, args_size);
    client->response->cgi_running = true;
}

void parse_cgi_output(t_client* client)
{
    int cgi_out = open(client->request->cgi_out.c_str(), O_RDONLY), rbytes = 0;
    if (cgi_out < 0)
    {
        fill_response(client, 501, "Internal Server Error", true);
        std::cerr << RED_BOLD <<"error: opening cgi output's file is failed!" << WHITE << std::endl;
        client->state = SERVED;
        return ;
    }
    char buff[MAX_BUFFER_SIZE];
    send(client->fd, "HTTP/1.1", 8, 0);
    rbytes = read(cgi_out, buff, MAX_BUFFER_SIZE);
    if (rbytes)
    {
        std::string str(buff, rbytes);
        size_t epos = str.find("\r\n");
        std::string line(str, 0, epos + 2);
        size_t spos = line.find("Status:");
        if(spos == std::string::npos)
            send(client->fd, " 200 OK\r\n", 9, 0);
        else
        {
            line.erase(0, 7);
            send(client->fd, line.c_str(), line.size(), 0);
            str.erase(0, epos + 2);
        }
        send(client->fd, str.c_str(), str.size(), 0);
    }

    // removed later on
    while(true)
    {
        rbytes = read(cgi_out, buff, MAX_BUFFER_SIZE);
        if (!rbytes)
            break;
        send(client->fd, buff, rbytes, 0);
    }
    client->state = SERVED;
    close(cgi_out);
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
        a += (isspace(s[a])), b -= (isspace(s[b]));
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
    long   res = 0;
    struct dirent *entry;

    dir = opendir(foldername);
    if (!dir)
        return (0) ;
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

std::string get_cache_name(std::string requested_uri)
{
    size_t      pos;
    std::string cache_name;

    pos = requested_uri.find("/");
    while (pos != std::string::npos)
    {
        requested_uri = requested_uri.substr(0, pos) + requested_uri.substr(pos + 1);
        pos = requested_uri.find("/");
    }
    cache_name = std::string("cache_") + requested_uri;
    return cache_name;
}