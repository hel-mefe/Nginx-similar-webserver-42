# include "../includes/parser.class.hpp"

ConfigFileParser::ConfigFileParser()
{
    return ;
}

ConfigFileParser::ConfigFileParser(const ConfigFileParser& p)
{
    (void)p;
    return ;
}

ConfigFileParser::~ConfigFileParser()
{
    return ;
}

void    ConfigFileParser::extract_lines_from_file(std::fstream &f_stream, std::vector<std::string> &lines)
{
    while (f_stream.good())
    {
        std::string line;
    
        std::getline(f_stream, line);
        lines.push_back(line);
    }
}


std::vector<std::string>    ConfigFileParser::get_word_vector(std::string &line)
{
    std::vector<std::string> word;
    bool flag = false;
    int last = 0, i = 0;

    for (; i < (int)line.size(); i++)
    {
        if (isspace(line[i]) || line[i] == '}' || line[i] == '{')
        {
            if (flag)
            {
                std::string w = line.substr(last, (i - last));
                word.push_back(w);
                flag = false;
            }
            if (line[i] == '}' || line[i] == '{')
            {
                std::string w;
                w += line[i];
                word.push_back(w);
            }
        }
        else if (!isspace(line[i]) && !flag)
            flag = true, last = i;
    }
    if (flag)
    {
        std::string w = line.substr(last, i - last);
        word.push_back(w);
    }
    return (word);
}

void    ConfigFileParser::extract_words_from_lines(std::vector<std::vector<std::string> > &words)
{
    std::vector<std::string> word;

    for (int i = 0; i < (int)lines.size(); i++)
    {
        word = get_word_vector(lines[i]);
        if (word.size()) // if the line is empty it will get ignored
            words.push_back(word);
        word.clear();
    }
}

bool is_brackets_correct(std::vector<std::vector<std::string> > &words)
{
    std::stack<char> st;

    for (int i = 0; i < sz(words); i++)
    {
        for (int j = 0; j < sz(words[i]); j++)
        {
            if (words[i][j] == "{")
                st.push('{');
            else if (words[i][j] == "}")
            {
                if (st.empty() || st.top() != '{')
                    return false ;
                st.pop();
            }
        }
    }
    return st.empty();
}

bool is_bracket(std::string &s)
{
    return (s == "{" || s == "}");
}

void    ConfigFileParser::build_brackets_queue()
{
    for (int i = 0; i < sz(words); i++)
    {
        for (int j = 0; j < sz(words[i]); j++)
        {
            if (is_bracket(words[i][j]))
                brackets_q.push_back(std::make_pair(words[i][j], std::make_pair(i, j)));
        }
    }
}

std::string    ConfigFileParser::get_identifier(std::pair<int, int> &start, std::pair<int, int> &end)
{
    std::string id;

    int i = start.first, end_i = end.first;
    int j = start.second + is_bracket(words[start.first][start.second]), end_j = end.second;
    i = end_i;
    j = end_j;
    bool found = false;
    while (i >= start.first && !found)
    {
        if (i == end_i)
            j = end_j;
        else
            j = sz(words[i]) - 1;
        while (j >= 0 && !found)
        {
            if (words[i][j] != "{" && words[i][j] != "}")
                id = words[i][j], found = true;
            j--;
        }
        i--;
    }
    return id;
}

void    ConfigFileParser::fill_words(std::vector<std::vector<std::string> > &n_words, \
std::pair<int, int> &start, std::pair<int, int> &end)
{
    int i = start.first;
    int j = start.second + 1, end_j = end.second;

    while (i < end.first)
    {
        std::vector<std::string> line_words;
        while (j < sz(words[i]))
        {
            if (i == end.first && j >= end_j)
                break ;
            line_words.push_back(words[i][j]);
            j++;
        }
        if (sz(line_words))
            n_words.push_back(line_words);
        j = 0;
        i++;
    }
}

void    ConfigFileParser::push_node(std::pair<int, int> prev, std::pair<int, int> &mid, std::pair<int, int> &end)
{
    t_node node;

    node.id = get_identifier(prev, mid);
    if (!node.id.length())
    {
        std::cerr << "[Webserv]: parsing error." << std::endl;
        exit(2);
    }
    fill_words(node.words, mid, end);
    nodes.push_back(node);
}

void    ConfigFileParser::build_with_vector()
{
    std::pair<int, int> first_all = std::make_pair(0, 0);

    push_node(first_all, brackets_q[0].second, brackets_q[sz(brackets_q) - 1].second);
    for (int i = 1; i < sz(brackets_q) - 1; i += 2)
        push_node(brackets_q[i - 1].second, brackets_q[i].second, brackets_q[i + 1].second);
}

void    ConfigFileParser::parse_http_configs(int &i, int &j)
{
    if (i >= sz(words))
        return ;
    int http_index = sz(http_as_words);
    while (i < sz(words))
    {
        std::vector<std::string> line_words;
        http_as_words.push_back(line_words);
        while (j < sz(words[i]) && !is_bracket(words[i][j]))
        {
            if (j < sz(words[i]) - 1 && words[i][j + 1] == "{")
                break ;
            else if (i < sz(words) && words[i + 1][0] == "{")
                break ;
            http_as_words[http_index].push_back(words[i][j]);
            j++;
        }
        if (!sz(http_as_words[sz(http_as_words) - 1]))
            http_as_words.pop_back() ;
        if ((j < sz(words[i]) && is_bracket(words[i][j])) || \
        (j < sz(words[i]) - 1 && is_bracket(words[i][j + 1])))
            break ;
        j = 0;
        http_index++;
        i++;
    }
}

void    ConfigFileParser::parse_server_location(t_node &node, int &i, int &j)
{
    while (i < sz(words))
    {
        std::vector<std::string>    location_block;
        while (j < sz(words[i]) && !is_bracket(words[i][j]))
        {
            location_block.push_back(words[i][j]);
            j++;
        }
        if (sz(location_block))
            node.location_blocks.push_back(location_block);
        if (j < sz(words[i]) && words[i][j] == "}")
            break ;
        j = 0;
        i++;
    }
}

void    ConfigFileParser::parse_server(int &i, int &j)
{
    t_node node;
    std::string id;

    bool flag = true;
    int a = i, b = j;
    if (!is_bracket(words[i][j]))
        id = words[i][j], j += 1;
    else
    {
        while (a >= 0 && flag)
        {
            b = sz(words[a]) - 1;
            while (b >= 0 && flag)
            {
                if (!is_bracket(words[a][b]))
                    id = words[a][b], flag = false;
                b--;
            }
            b = 0;
            a--;
        }
    }
    while (i < sz(words))
    {
        std::vector<std::string> line_words;
        while (j < sz(words[i]) && words[i][j] != "}")
        {
            if (words[i][j] == "location")
                parse_server_location(node, i, j);
            if (!is_bracket(words[i][j]))
            {
                if (!sz(node.id))
                    node.id = id;
                line_words.push_back(words[i][j]);
            }
            j++;
        }
        if (sz(line_words))
            node.words.push_back(line_words);
        if (j < sz(words[i]) && words[i][j] == "}")
            break ;
        i++;
        j = 0;
    }
    if (sz(node.id))
        nodes.push_back(node);
}

void    ConfigFileParser::parse_words()
{
    int i = 0, j = 0;
    if (!sz(words))
        return ;
    if (sz(words[i]) > 1)
        i = 1, j = 0;
    else if (sz(words[i]) == 1)
        i = 1, j = 1;
    while (i < sz(words))
    {
        while (j < sz(words[i]))
        {
            while (j < sz(words[i]) && is_bracket(words[i][j]))
                j++;
            parse_http_configs(i, j);
            if (i >= sz(words))
                break ;
            parse_server(i, j);
            if (i >= sz(words))
                break ;
            while (j < sz(words[i]) && is_bracket(words[i][j]))
                j++;
            j++;
        }
        j = 0;
        i++;
    }
}

/*** Tokens filling 
 * 
 * the following functions are responsible for defining all the config file tokens
 * http blocks have their independent tokens
 * server blocks have their independent tokens
 * location blocks have their independent tokens
 ***/

void    ConfigFileParser::fill_http_hashmap()
{
    http_tokens.insert(std::make_pair("auto_indexing", ON_OFF));
    http_tokens.insert(std::make_pair("allowed_methods", METHOD_VECTOR));
    http_tokens.insert(std::make_pair("root", DIRECTORY));
    http_tokens.insert(std::make_pair("max_connections", INT));
    http_tokens.insert(std::make_pair("max_body_size", INT));
    http_tokens.insert(std::make_pair("connection", CONNECTION));
    http_tokens.insert(std::make_pair("max_request_timeout", INT));
    http_tokens.insert(std::make_pair("multiplexer", MULTIP));
    http_tokens.insert(std::make_pair("fastCGI", CGI));
}

void    ConfigFileParser::fill_server_hashmap()
{
    server_tokens.insert(std::make_pair("root", DIRECTORY));
    server_tokens.insert(std::make_pair("auto_indexing", ON_OFF));
    server_tokens.insert(std::make_pair("max_connections", INT));
    server_tokens.insert(std::make_pair("connection", CONNECTION));
    server_tokens.insert(std::make_pair("server_name", STRING));
    server_tokens.insert(std::make_pair("port", SHORT_INT));
    server_tokens.insert(std::make_pair("location", DIRECTORY));
    server_tokens.insert(std::make_pair("index", STRING_VECTOR));
    server_tokens.insert(std::make_pair("not_found", STRING_VECTOR));
    server_tokens.insert(std::make_pair("allowed_methods", STRING_VECTOR));
    server_tokens.insert(std::make_pair("max_body_size", INT));
    server_tokens.insert(std::make_pair("fastCGI", CGI));
    server_tokens.insert(std::make_pair("error_page", ERROR_PAGE));
    server_tokens.insert(std::make_pair("register_logs", STRING));
    server_tokens.insert(std::make_pair("max_request_timeout", INT));
}

void    ConfigFileParser::fill_location_hashmap()
{
    location_tokens.insert(std::make_pair("root", DIRECTORY));
    location_tokens.insert(std::make_pair("allowed_methods", STRING_VECTOR));
    location_tokens.insert(std::make_pair("auto_indexing", ON_OFF));
    location_tokens.insert(std::make_pair("connection", CONNECTION));
    location_tokens.insert(std::make_pair("server_name", STRING));
    location_tokens.insert(std::make_pair("port", SHORT_INT));
    location_tokens.insert(std::make_pair("location", DIRECTORY));
    location_tokens.insert(std::make_pair("index", STRING_VECTOR));
    location_tokens.insert(std::make_pair("not_found", STRING_VECTOR));
    location_tokens.insert(std::make_pair("redirect", STRING));
    location_tokens.insert(std::make_pair("upload", ON_OFF)); // off is the dafault value
    location_tokens.insert(std::make_pair("directory_listing", ON_OFF));
    location_tokens.insert(std::make_pair("location", LOCATION));
    location_tokens.insert(std::make_pair("error_page", ERROR_PAGE));

}

void    ConfigFileParser::fill_tokens()
{
    fill_http_hashmap();
    fill_server_hashmap();
    fill_location_hashmap();
}

/*** Config file validity checking functions ***
 * 
 * all of these functions are checking if the config file is valid or not
 * before parsing the config file and extracting all the data there is one iteration O(N)
 * that should be done to check all the lines if valid or not
***/

bool ConfigFileParser::is_http_line_valid(int row)
{
    std::string token_name;
    TOKEN       token_type;

    token_name = http_as_words[row][0];
    if (sz(token_name) && token_name[0] == '#') // comment
        return true ;
    if (http_tokens.find(token_name) == http_tokens.end())
        return (false) ;
    token_type = http_tokens[token_name];
    if ((token_type == DIRECTORY || token_type == MULTIP || token_type == SHORT_INT || token_type == INT || token_type == ON_OFF ||\
    token_type == METHOD || token_type == STRING) && (sz(http_as_words[row]) != 2))
        return (false);
    else if ((token_type == METHOD_VECTOR || token_type == STRING_VECTOR) && (sz(http_as_words[row]) < 2))
        return (false);
    if (token_type == DIRECTORY)
        return (tc.is_directory(http_as_words[row][1]));
    else if (token_type == SHORT_INT)
        return (tc.is_short_int(http_as_words[row][1]));
    else if (token_type == INT)
        return (tc.is_int(http_as_words[row][1]));
    else if (token_type == METHOD)
        return (tc.is_method(http_as_words[row][1]));
    else if (token_type == METHOD_VECTOR)
    {
        for (int i = 1; i < sz(http_as_words[row]); i++)
        {
            if (!tc.is_method(http_as_words[row][i]))
                return (false);
        }
    }
    else if (token_type == ON_OFF)
        return (tc.is_on_off(http_as_words[row][1]));
    else if (token_type == CGI)
        return (tc.is_cgi(http_as_words[row][1], http_as_words[row][2]));
    else if (token_type == MULTIP)
        return (tc.is_multiplexer(http_as_words[row][1]));
    return (true);
}

bool ConfigFileParser::is_http_valid()
{
    int i = 0, j = 0;
    while (i < sz(http_as_words))
    {
        j = 0;
        if (!is_http_line_valid(i))
            return (false) ;
        i++;
    }
    return (true);
}

bool ConfigFileParser::is_server_line_valid(std::vector<std::string> &_words)
{
    std::string token_name = _words[0];
    TOKEN       token_type;

    if (sz(token_name) && token_name[0] == '#')
        return true ;
    if (server_tokens.find(token_name) == server_tokens.end())
    {
        std::cerr << RED_BOLD << "Invalid token in the config file" << std::endl;
        return (false);
    }
    token_type = server_tokens[token_name];
    if ((token_type == STRING || token_type == METHOD || token_type == INT || token_type == SHORT_INT \
    || token_type == DIRECTORY || token_type == CONNECTION) && (sz(_words) != 2))
    {
        std::cerr << RED_BOLD << "syntax error in the config file" << std::endl;
        return (false);
    }
    else if ((token_type == METHOD_VECTOR || token_type == STRING_VECTOR) && (sz(_words) < 2))
    {
        if (token_type == METHOD_VECTOR)
            std::cerr << RED_BOLD << "syntax error in the list of the provided methods" << std::endl;
        else
            std::cerr << RED_BOLD << "syntax error in the config file" << std::endl;
        return (false);
    }
    else if ((token_type == CGI || token_type == ERROR_PAGE) && sz(_words) != 3)
    {
        if (token_type == CGI)
            std::cerr << RED_BOLD << "cgi token syntax error!" << std::endl;
        else
            std::cout << RED_BOLD << "error_pages token syntax error" << std::endl;
        return (false);
    }
    if (token_type == DIRECTORY)
        return (tc.is_directory(_words[1]));
    else if (token_type == SHORT_INT)
    {
        if (!tc.is_short_int(_words[1]))
            std::cerr << RED_BOLD << "Invalid port" << std::endl;
        return (tc.is_short_int(_words[1]));
    }
    else if (token_type == INT)
        return (tc.is_int(_words[1]));
    else if (token_type == METHOD)
    {
        if (!tc.is_method(_words[1]))
            std::cerr << RED_BOLD << "Invalid method" << std::endl;
        return (tc.is_method(_words[1]));
    }
    else if (token_type == CONNECTION)
        return (tc.is_connection(_words[1]));
    else if (token_type == METHOD_VECTOR)
    {
        for (int i = 1; i < sz(_words); i++)
        {
            if (!tc.is_method(_words[i]))
            {
                std::cerr << RED_BOLD << "Invalid method" << std::endl;
                return (false);
            }
        }
    }
    else if (token_type == ON_OFF)
        return (tc.is_on_off(_words[1]));
    else if (token_type == CGI)
        return (tc.is_cgi(_words[1], _words[2]));
    else if (token_type == ERROR_PAGE)
        return (tc.is_code(_words[1])) ;
    return (true);
}

bool ConfigFileParser::is_location_block_valid(std::vector<std::string> &block)
{
    std::string token_name = block[0];
    TOKEN       token_type;

    if (sz(token_name) && token_name[0] == '#')
        return (true) ;
    if (location_tokens.find(token_name) == location_tokens.end())
    {
        std::cerr << RED_BOLD << "Invalid token in the config file" << std::endl;
        return (false);
    }
    token_type = location_tokens[token_name];
    if ((token_type == STRING || token_type == METHOD || token_type == INT || token_type == SHORT_INT \
    || token_type == DIRECTORY || token_type == ON_OFF || token_type == CONNECTION) && (sz(block) != 2))
    {
        std::cerr << RED_BOLD << "syntax error in the config file" << std::endl;
        return (false);
    }
    else if ((token_type == METHOD_VECTOR || token_type == STRING_VECTOR) && (sz(block) < 2))
    {
        if (token_type == METHOD_VECTOR)
            std::cerr << RED_BOLD << "syntax error in the list of the provided methods" << std::endl;
        else
            std::cerr << RED_BOLD << "syntax error in the config file" << std::endl;
        return (false);
    }
    else if ((token_type == CGI || token_type == ERROR_PAGE) && sz(block) != 3)
    {
        if (token_type == CGI)
            std::cerr << RED_BOLD << "cgi token syntax error!" << std::endl;
        else
            std::cout << RED_BOLD << "error_pages token syntax error" << std::endl;
        return (false) ;
    }
    if (token_type == DIRECTORY)
        return (tc.is_directory(block[1]));
    else if (token_type == SHORT_INT)
        return (tc.is_short_int(block[1]));
    else if (token_type == INT)
        return (tc.is_int(block[1]));
    else if (token_type == METHOD)
    {
        if (!tc.is_method(block[1]))
            std::cerr << RED_BOLD << "Invalid method" << std::endl;
        return (tc.is_method(block[1]));
    }
    else if (token_type == CONNECTION)
        return (tc.is_connection(block[1]));
    else if (token_type == METHOD_VECTOR)
    {
        for (int i = 1; i < sz(block); i++)
        {
            if (!tc.is_method(block[i]))
            {
                std::cerr << RED_BOLD << "Invalid method" << std::endl;
                return (false);
            }
        }
    }
    else if (token_type == ON_OFF)
        return (tc.is_on_off(block[1]));
    else if (token_type == CGI)
        return (tc.is_cgi(block[1], block[2]));
    else if (token_type == ERROR_PAGE)
        return (tc.is_code(block[1]));
    return (true);
}

bool ConfigFileParser::is_servers_valid()
{
    for (int i = 0; i < sz(nodes); i++)
    {
        for (int j = 0; j < sz(nodes[i].words); j++)
        {
            if (!is_server_line_valid(nodes[i].words[j]))
            {
                std::cerr << RED_BOLD << "(syntax error at server word -> " << j << ")" << std::endl ;
                return (false);
            }
        }
        for (int k = 0; k < sz(nodes[i].location_blocks); k++)
        {
            if (!is_location_block_valid(nodes[i].location_blocks[k]))
            {
                std::cerr << RED_BOLD << "(syntax error at location block line -> " << k << ")" << std::endl ;
                return (false) ;
            }
        }
    }
    return (true);
}

int ConfigFileParser::count_words()
{
    int i = 0, res = 0;
    while (i < sz(words))
    {
        int j = 0;
        while (j < sz(words[i]))
        {
            res++;
            j++;
        }
        i++;
    }
    return (res);
}

bool ConfigFileParser::is_config_file_valid(std::string &config_file) // checks if the config file syntax is valid or not
{
    std::fstream c_stream;
    bool        is_everything_valid;
    c_stream.open(config_file, std::ios::in);
    if (!c_stream.good())
        return (false);
    fill_tokens();
    extract_lines_from_file(c_stream, lines);
    extract_words_from_lines(words);
    if (!is_brackets_correct(words))
    {
        c_stream.close();
        return (false);
    }
    build_brackets_queue();
    std::vector<std::string> k;
    int words_size = count_words();
    if (words_size < 3 || (words_size > 3 && words[0][0] != "http")) // 1 -> http | 2 -> { 3-> }
    {
        c_stream.close();
        return (false) ;
    }
    parse_words();
    is_everything_valid = (is_http_valid() && is_servers_valid());
    c_stream.close();
    return (is_everything_valid) ;
}

void    ConfigFileParser::insert_cgi_to_hashmap(HashMap<std::string, std::string> &extension_cgi, std::vector<std::string> &line)
{
    std::string extension = line[1];
    std::string cgi_path = line[2];
    std::string fullpath = getwd(NULL);

    fullpath += cgi_path;
    if (access(fullpath.c_str(), X_OK))
    {
        std::cout << "INSERTING ERROR " << std::endl;
        throw ParsingExceptionCgi();
    }
    extension_cgi[extension] = cgi_path;
}

int     get_max_connections(std::string &s)
{
    int num = std::atoi(s.c_str());

    if (num <= 0 || num > DEFAULT_MAX_CONNECTIONS)
        throw ParsingExceptionMaxConnectionsInterval() ;
    return (num) ;
}

int     get_max_body_size(std::string &s)
{
    long long num = std::stoll(s);

    if (num < 0 || num >= INT_MAX)
        throw ParsingException();
    return (num);
}

/*** Filling functions 
 * 
 * all of this functions are responsible for filling the data 
 ****/

void    ConfigFileParser::fill_server_attributes(t_server_configs &attr, t_http_configs *conf, int i)
{
    attr.allowed_methods = conf->allowed_methods;
    attr.allowed_methods_set = conf->allowed_methods_set;
    attr.max_body_size = conf->max_body_size;
    attr.root = conf->root;
    attr.max_request_timeout = conf->max_request_timeout;
    for (int j = 0; j < sz(nodes[i].words); j++)
    {
        if (nodes[i].id != "server")
            throw InvalidHttpToken() ;
        std::string token_name = nodes[i].words[j][0];
        if (token_name == "auto_indexing")
            attr.auto_indexing = get_auto_indexing(nodes[i].words[j]);
        else if (token_name == "connection")
            attr.connection = get_connection(nodes[i].words[j]);
        else if (token_name == "server_name")
            attr.server_name = nodes[i].words[j][1];
        else if (token_name == "port")
        {
            attr.port = get_port(nodes[i].words[j]);
            if (ports_set.find(attr.port) != ports_set.end())
                throw PortAlreadyUsed();
            ports_set.insert(attr.port);
        }
        else if (token_name == "index")
        {
            attr.indexes = get_vector_of_data(nodes[i].words[j]);
            attr.indexes_set = vector_to_hashset(attr.indexes);
        }
        else if (token_name == "allowed_methods")
        {
            attr.allowed_methods = get_vector_of_data(nodes[i].words[j]);
            for (int k = 0; k < sz(attr.allowed_methods); k++) // checking if all methods are valid
            {
                if (!tc.is_method(attr.allowed_methods[k]))
                    throw InvalidMethod();
            }
            attr.allowed_methods_set  = vector_to_hashset(attr.allowed_methods);
        }
        else if (token_name == "root")
        {
            attr.root = conf->root + nodes[i].words[j][1];
            attr.root = (attr.root[sz(attr.root) - 1] != '/') ? attr.root + "/" : attr.root;
        }
        else if (token_name == "max_connections")
            attr.max_connections = get_max_connections(nodes[i].words[j][1]); // will crash in case passed MAX_CONNECTIONS_ALLOWED 1024
        else if (token_name == "max_body_size")
            attr.max_body_size = get_max_body_size(nodes[i].words[j][1]); // will crash in case passed MAX_INT
        else if (token_name == "fastCGI")
            insert_cgi_to_hashmap(attr.extension_cgi, nodes[i].words[j]);
        else if (token_name == "not_found")
        {
            attr.pages_404 = get_vector_of_data(nodes[i].words[j]);
            attr.pages_404_set = vector_to_hashset(attr.pages_404);
        }
        else if (token_name == "register_logs")
        {
            attr.logsfile = nodes[i].words[j][1];
            if (attr.logsfile_fd != UNDEFINED)
                close(attr.logsfile_fd);
            attr.logsfile_fd = open(attr.logsfile.c_str(), O_CREAT | O_APPEND, O_RDWR);
        }
        else if (token_name == "error_page")
        {
            int code = std::atoi(nodes[i].words[j][1].c_str());
            attr.code_to_page[code] = nodes[i].words[j][2];
        }
        else if (token_name == "max_request_timeout")
            attr.max_request_timeout = std::atoi(nodes[i].words[j][1].c_str());
    }
    if (attr.code_to_page.find(404) != attr.code_to_page.end())
        attr.pages_404.push_back(attr.code_to_page[404]);
}

bool ConfigFileParser::get_auto_indexing(std::vector<std::string> &line)
{
    return (line[1] == "on");
}

bool ConfigFileParser::get_connection(std::vector<std::string> &line)
{
    return (line[1] == "keep-alive");
}

int ConfigFileParser::get_port(std::vector<std::string> &line)
{
    return (std::atoi(line[1].c_str()));
}

std::vector<std::string> ConfigFileParser::get_vector_of_data(std::vector<std::string> &line)
{
    std::vector<std::string> data;
    HashSet<std::string>     keep;

    for (int i = 1; i < sz(line); i++)
    {
        if (keep.find(line[i]) == keep.end())
        {
            data.push_back(line[i]);
            keep.insert(line[i]);
        }
    }
    return (data);
}

HashSet<std::string>    ConfigFileParser::vector_to_hashset(std::vector<std::string> &vec)
{
    HashSet<std::string> res;

    for (int i = 0; i < sz(vec); i++)
        res.insert(vec[i]);
    return (res);
}

void    ConfigFileParser::normalize_directories_vector(std::vector<std::string> &vec)
{
    for (int i = 0; i < sz(vec); i++)
    {
        if (vec[i][sz(vec[i]) - 1] != '/')
            vec[i] += "/" ;
    }
}


void    ConfigFileParser::fill_location_attributes(t_location_configs &l_configs, int i, int j, int ej)
{
    while (j < ej && j < sz(nodes[i].location_blocks))
    {
        std::string token_name = nodes[i].location_blocks[j][0];
        if (token_name == "auto_indexing")
            l_configs.auto_indexing = get_auto_indexing(nodes[i].location_blocks[j]);
        else if (token_name == "connection")
            l_configs.connection = get_connection(nodes[i].location_blocks[j]);
        else if (token_name == "index")
        {
            l_configs.indexes = get_vector_of_data(nodes[i].location_blocks[j]);
            l_configs.indexes_set = vector_to_hashset(l_configs.indexes);
        }
        else if (token_name == "allowed_methods")
        {
            l_configs.allowed_methods = get_vector_of_data(nodes[i].location_blocks[j]);
            for (int j = 0; j < sz(l_configs.allowed_methods); j++) // checking if all methods are valid
            {
                if (!tc.is_method(l_configs.allowed_methods[j]))
                    throw InvalidMethod();
            }
            l_configs.allowed_methods_set = vector_to_hashset(l_configs.allowed_methods);
        }
        else if (token_name == "root")
        {
            l_configs.root = nodes[i].location_blocks[j][1]; // [!! -- Later on I may add root + nodes[i].location_blocks[j][1] -- ]
            l_configs.root = (l_configs.root[sz(l_configs.root) - 1] != '/') ? l_configs.root + "/" : l_configs.root;
        }
        else if (token_name == "redirect")
            l_configs.redirection = nodes[i].location_blocks[j][1];
        else if (token_name == "not_found")
        {
            l_configs.pages_404 = get_vector_of_data(nodes[i].location_blocks[j]);
            l_configs.pages_404_set = vector_to_hashset(l_configs.pages_404);
        }
        else if (token_name == "upload")
            l_configs.upload = get_auto_indexing(nodes[i].location_blocks[j]);
        else if (token_name == "directory_listing")
            l_configs.directory_listing = get_auto_indexing(nodes[i].location_blocks[j]);
        else if (token_name == "error_page")
        {
            int code = std::atoi(nodes[i].location_blocks[j][1].c_str());
            std::string page = nodes[i].location_blocks[j][2];
            l_configs.code_to_page[code] = page;
        }
        j++;
    }
    if (l_configs.code_to_page.find(404) != l_configs.code_to_page.end())
        l_configs.pages_404.push_back(l_configs.code_to_page[404]);

}

void    ConfigFileParser::handle_locations(t_server *server, std::vector<std::vector<std::string> > &location_blocks, t_server_configs *s_conf, int index)
{
    int i = 0;
    while (i < sz(location_blocks) && sz(location_blocks[i]) && location_blocks[i][0] != "location")
        i++;
    while (i < sz(location_blocks))
    {
        t_location_configs *conf = new t_location_configs();
        std::string location = location_blocks[i][1];
        if (location[sz(location) - 1] != '/')
            location += "/";
        i++;
        int start = i;
        while (i < sz(location_blocks) && location_blocks[i][0] != "location")
            i++;
        conf->allowed_methods = s_conf->allowed_methods;
        conf->allowed_methods_set = s_conf->allowed_methods_set;
        conf->indexes = s_conf->indexes;
        conf->indexes_set = s_conf->indexes_set;
        conf->pages_404 = s_conf->pages_404;
        conf->pages_404_set = s_conf->pages_404_set;
        conf->code_to_page = s_conf->code_to_page;
        fill_location_attributes(*conf, index, start, i);
        server->set_location_map(location, conf);
    }
}

bool ConfigFileParser::fill_servers_data(std::vector<t_server *> *servers, t_http_configs *conf)
{
    HashSet<std::string> already_parsed;

    for (int i = 0; i < sz(nodes); i++)
    {
        t_server      *server = new t_server();
        t_server_configs *attr = new t_server_configs();
        std::string     location_name;

        fill_server_attributes(*attr, conf, i);
        handle_locations(server, nodes[i].location_blocks, attr, i);
        server->set_server_configs(attr);
        servers->push_back(server);
    }
    return (true);
}

bool ConfigFileParser::fill_http_data(t_http_configs *http_data)
{
    HashSet<std::string> already_parsed;

    http_data->max_body_size = 1000000; //1mb
    for (int i = 0; i < sz(http_as_words); i++)
    {
        std::string token_name = http_as_words[i][0];
        if (token_name == "auto_indexing")
            http_data->auto_indexing = get_auto_indexing(http_as_words[i]);
        else if (token_name == "root")
            http_data->root = http_as_words[i][1];
        else if (token_name == "allowed_methods")
        {
            http_data->allowed_methods = get_vector_of_data(http_as_words[i]);
            for (int j = 0; j < sz(http_data->allowed_methods); j++) // checking if all methods are valid
            {
                if (!tc.is_method(http_data->allowed_methods[j]))
                    throw InvalidMethod();
            }
            http_data->allowed_methods_set = vector_to_hashset(http_data->allowed_methods);
        }
        else if (token_name == "max_body_size")
            http_data->max_body_size = std::atoi(http_as_words[i][1].c_str());
        else if (token_name == "max_request_timeout")
            http_data->max_request_timeout = std::atoi(http_as_words[i][1].c_str());
    }
    return (true);
}

bool ConfigFileParser::parse_config_file(std::string config_file, t_http_configs *http_data, std::vector<t_server *> *servers)
{
    if (!is_config_file_valid(config_file)) // O(N) to check if the file is valid or not
    {
        std::cerr << CYAN_BOLD << "[Webserv42]: config file is not valid" << std::endl;
        return (false);
    }
    return (fill_http_data(http_data) && fill_servers_data(servers, http_data));
}