# include "../inc/http_parser.class.hpp"

/*** START PARSING PART ***/

bool    HttpParser::read_header(t_client *client)
{
    char buff[MAX_BUFFER_SIZE];
    int bytes; 
    //add buffer to request class to append buff everytime u entered this function u didnt finish header

    bytes = recv(client->fd, buff, MAX_BUFFER_SIZE, 0);
    if (bytes <= 0)
    {
        if (!bytes) // bytes == 0 connection closed
            client->state = SERVED;
        return (false);
    }
    int bl = 0, el = 0;
    while (el < bytes)
    {
        if (buff[el] == '\r' && buff[el + 1] == '\n')
        {
            if (el - bl)
                client->request->lines.push_back(std::string(buff + bl, el - bl));
            else
            {
                if (buff + (el + 2))
                    client->request->body.append(buff + (el + 2), bytes - (el + 2));
                return true;
            }
            bl = el+=2;
        }
        else
            el++;
    }
    return (false);
}

bool    HttpParser::parse_first_line(t_request *req)
{
    std::string line = req->lines[0];
    std::string method, path, http_version;
    int i = 0, k = 0;
    std::vector<std::string> *splitted = split_first_line(line);

    req->method = get_upper_case(splitted->at(0));
    std::cout << RED_BOLD << req->method << std::endl;
    req->path = splitted->at(1);
    std::cout << RED_BOLD << "PATH IN REQUEST -> " << req->path << std::endl;
    req->http_version = get_upper_case(splitted->at(2));
    req->is_file = is_file(req->path);
    if (req->is_file)
    {
        req->extension = get_extension(req->path);
        req->filename = get_filename(req->path);
    }
    delete splitted;
    return ((req->method == "GET" || req->method == "POST" || req->method == "DELETE") && (req->http_version == "HTTP/1.1"));

}

bool    HttpParser::parse_request(t_client *client)
{
    t_request *req;

    req = client->request;
    if (!parse_first_line(req))
        return (false) ;
    std::cout << "PARSE FIRST LINE IS VALID" << std::endl;
    for (int i = 1; i < sz(req->lines); i++)
    {
        std::string line = req->lines[i], first, second;

        if (sz(line) < 3)
            return (false) ;
        size_t  spoint = line.find_first_of(":");
        first = line.substr(0, spoint);
        first = trim_string(first);
        second = line.substr(spoint);
        second = trim_string(second);
        if (!sz(first) || !sz(second))
            return (false) ;
        // int j = 0, a = 0;
        // for (; j < sz(line) && isspace(line[j]); j++);
        // a = j;
        // for (; j < sz(line) && line[j] != ':'; j++);
        // first = get_lower_case(line.substr(a, j - a));
        // for (; j < sz(line) && isspace(line[j]); j++);
        // a = j + (line[j]== ':') + 1;
        // for (; j < sz(line) && line[j] != '\r'; j++);
        // second = get_lower_case(line.substr(a, j - a));
        // first = trim_string(first);
        // second = trim_string(second);
        if (first == "cookie")
        {
            if (!req->cookies.empty())
                req->cookies.append("; ");
            req->cookies.append(second);
        }
        else
            req->request_map.insert(std::make_pair(first, second));
    }
    std::cout << "REQUEST IS VALID" << std::endl;
    return (true) ;
}

/*** END PARSING PART ***/


