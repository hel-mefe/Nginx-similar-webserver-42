# include "http_parser.class.hpp"

/*** START PARSING PART ***/

bool    HttpParser::read_header(t_client *client)
{
    char buff[MAX_BUFFER_SIZE];
    int bytes;
    std::string lines;
    bool header_complete = false ;

    bzero(buff, MAX_BUFFER_SIZE);
    bytes = read(client->fd, buff, MAX_BUFFER_SIZE);
    if (bytes == -1)
        return (false);
    else if (!bytes)
    { // bytes == 0 connection closed
        client->state = SERVED;
        return (false);
    }
    lines = buff;
    client->request_time = time(NULL);
    std::stringstream strm(lines);
    while (strm.good() && !header_complete)
    {
        std::string line;
        std::getline(strm, line);
        if (line[sz(line) - 1] == '\r')
            line = line.substr(0, sz(line) - 1);
        header_complete = !sz(line);
        if (sz(line) && !header_complete)
            client->request->lines.push_back(line);
    }
    return (header_complete);
}

void    HttpParser::parse_first_line(t_request *req)
{
    std::string line = req->lines[0];
    std::string method, path, http_version;
    int i = 0, k = 0;
    std::vector<std::string> *splitted = split_first_line(line);

    req->method = get_upper_case(splitted->at(0));
    req->path = splitted->at(1);
    // std::cout << RED_BOLD << "PATH IN REQUEST -> " << req->path << std::endl;
    req->http_version = get_upper_case(splitted->at(2));
    req->is_file = is_file(req->path);
    if (req->is_file)
        req->extension = get_extension(req->path);
    delete splitted;
}

void    HttpParser::parse_request(t_client *client)
{
    t_request *req;

    req = client->request;
    parse_first_line(req);
    for (int i = 1; i < sz(req->lines); i++)
    {
        std::string line = req->lines[i], first, second;
    
        int j = 0, a = 0;
        for (; j < sz(line) && isspace(line[j]); j++);
        a = j;
        for (; j < sz(line) && line[j] != ':'; j++);
        first = get_lower_case(line.substr(a, j - a));
    
        for (; j < sz(line) && isspace(line[j]); j++);
        a = j;
        for (; j < sz(line) && line[j] != '\r'; j++);
        second = get_lower_case(line.substr(a, j - a));
        first = trim_string(first);
        second = trim_string(second);
        req->request_map.insert(std::make_pair(first, second));
    }
    /****** START PRINTING REQUEST ******/

    // std::cout << WHITE << "***** PRINTING REQUEST MAP *****" << std::endl;
    // for (auto x: req->request_map)
    //     std::cout << x.first << " " << x.second << std::endl;
    // std::cout << "***** END PRINTING REQUEST MAP *****" << std::endl;

    /****** END PRINTING REQUEST *****/
}

/*** END PARSING PART ***/

