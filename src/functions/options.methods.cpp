# include "../includes/options.class.hpp"

std::string Options::get_allowed_methods(t_client *client)
{
    t_request                   *req;
    std::vector<std::string>    asked_methods;
    std::vector<std::string>    allowed_methods;
    std::string                 smethods;
    std::string                 s;
    std::string                 ress;
    size_t                      pos;
    size_t                      start_pos;

    req = client->request;
    smethods = req->request_map["access-control-request-method"];
    pos = sz(smethods);
    allowed_methods = client->response->dir_configs ? client->response->dir_configs->allowed_methods : client->response->configs->allowed_methods;
    while (pos == smethods.size() || pos != std::string::npos)
    {
        start_pos = pos == smethods.size() ? 0 : pos + 1;
        pos = smethods.find(',', pos == smethods.size() ? 0 : pos + 1);
        s = pos == std::string::npos ? smethods.substr(start_pos) : smethods.substr(start_pos, pos - start_pos);
        s = get_upper_case(s);
        s = trim_string(s);
        asked_methods.push_back(s);        
    }
    for (int i = 0; i < sz(allowed_methods); i++)
        std::cout << allowed_methods[i] << " " << std::endl;
    for (int i = 0; i < sz(asked_methods); i++)
    {
        if (std::find(allowed_methods.begin(), allowed_methods.end(), asked_methods[i]) != allowed_methods.end())
            ress = (sz(ress)) ? ress + ", " + asked_methods[i] : asked_methods[i] ;
    }
    return (ress) ;
}

void    Options::serve_client(t_client *client)
{
    t_request   *req;
    std::string ress;

    req = client->request;
    ress = "HTTP/1.1 200 OK\r\n" ;
    if (IN_MAP(req->request_map, "origin"))
        ress += "access-control-allow-origin: " + req->request_map["origin"] + "\r\n";
    if (IN_MAP(req->request_map, "access-control-request-method"))
        ress += "access-control-allow-methods: " + get_allowed_methods(client) + "\r\n";
    if (IN_MAP(req->request_map, "access-control-request-headers"))
        ress += "access-control-allow-headers: Content-Type, Content-Length, Connection, Transfer-Encoding, Set-Cookie\r\n";
    if (IN_MAP(req->request_map, "access-control-max-age"))
        ress += "access-control-max-age: 0\r\n" ;
    if (IN_MAP(req->request_map, "access-control-credentials"))
    {
        ress += "access-contro-allow-credentials: ";
        ress += ((client->server->server_configs->cookies) ? "true\r\n" : "false\r\n");
    }
    ress += "\r\n";
    std::cout << GREEN_BOLD << ress << std::endl;
    send(client->fd, ress.c_str(), sz(ress), 0);
    client->state = SERVED;
}