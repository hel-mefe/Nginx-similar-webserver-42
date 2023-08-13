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
    delete splitted;
    return (IS_METHOD_SUPPORTED(req->method) && (req->http_version == "HTTP/1.1"));

}

int    HttpParser::parse_request(t_client *client)
{
    t_request *req;

    req = client->request;
    if (!parse_first_line(req))
    {
        if (req->http_version != "HTTP/1.1" || req->method == "QUERY" || req->method == "CONNECT")
            req->is_not_implemented = true;
        else
            req->is_bad_request = true;
        if (!IS_METHOD_SUPPORTED(req->method))
            req->method = "GET" ;
        return (0);
    }
    for (int i = 1; i < sz(req->lines); i++)
    {
        std::string line = req->lines[i], first, second;

        if (sz(line) < 3)
        {
            req->is_bad_request = true ;
            return (0) ;
        }
        size_t  spoint = line.find_first_of(":");
        first = line.substr(0, spoint);
        first = trim_string(first);
        second = line.substr(spoint + 1);
        second = trim_string(second);
        second = get_lower_case(second);
        first = get_lower_case(first);
        if (!sz(first) || !sz(second))
        {
            req->is_bad_request = true;
            return (0) ;
        }
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
        req->is_bad_request = true;
    return (0) ;
}

/*** END PARSING PART ***/


