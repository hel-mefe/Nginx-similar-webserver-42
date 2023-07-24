# include "../includes/http_parser.class.hpp"

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
        {
            std::cout << "READ 0 CHARS" << std::endl;
            client->state = SERVED;
        }
        return (false);
    }
    int bl = 0, el = 0;
    std::cout << "READING THE HEADER " << std::endl;
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
    std::vector<std::string> *splitted = split_first_line(line);

    req->method = get_upper_case(splitted->at(0));
    req->path = splitted->at(1);
    size_t pos = req->path.find("?");
    if (pos != std::string::npos)
    {
        req->queries = req->path.substr(pos + 1);
        req->path.erase(pos);
    }
    req->http_version = get_upper_case(splitted->at(2));
    req->is_file = is_file(req->path);
    if (req->is_file)
    {
        req->extension = get_extension(req->path);
        req->filename = get_filename(req->path);
    }
    delete splitted;
    return ((req->method == "GET" || req->method == "POST" || req->method == "DELETE" || req->method == "OPTIONS" || req->method == "HEAD" || req->method == "PUT") && (req->http_version == "HTTP/1.1"));

}

bool    HttpParser::parse_request(t_client *client)
{
    t_request *req;

    req = client->request;
    if (!parse_first_line(req))
        return (false) ;
    for (int i = 1; i < sz(req->lines); i++)
    {
        std::string line = req->lines[i], first, second;

        if (sz(line) < 3)
            return (false) ;
        size_t  spoint = line.find_first_of(":");
        first = line.substr(0, spoint);
        first = trim_string(first);
        second = line.substr(spoint + 1);
        second = trim_string(second);
        second = get_lower_case(second);
        first = get_lower_case(first);
        if (!sz(first) || !sz(second))
            return (false) ;
        if (first == "cookie" && client->server->server_configs->cookies)
        {
            if (!req->cookies.empty())
                req->cookies.append("; ");
            req->cookies.append(second);
        }
        else
            req->request_map.insert(std::make_pair(first, second));
    }
    if (req->method == "OPTIONS" && !IN_MAP(req->request_map, "origin"))
        return (false) ;
    return (true) ;
}

/*** END PARSING PART ***/


