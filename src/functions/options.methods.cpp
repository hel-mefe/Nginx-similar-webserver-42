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
    while (pos == sz(smethods) || pos != std::string::npos)
    {
        start_pos = pos == sz(smethods) ? 0 : pos + 1;
        pos = smethods.find(',', pos == sz(smethods) ? 0 : pos + 1);
        s = pos == std::string::npos ? smethods.substr(start_pos) : smethods.substr(start_pos, pos - start_pos);
        s = get_upper_case(s);
        asked_methods.push_back(s);        
    }
    for (int i = 0; i < sz(asked_methods); i++)
    {
        if (std::find(allowed_methods.begin(), allowed_methods.end(), asked_methods[i]) != allowed_methods.end())
            ress = (sz(ress)) ? ress + ", " + asked_methods[i] : asked_methods[i];
    }
    return (ress) ;
}

std::string Options::get_allowed_headers(t_client *client)
{
    t_request                   *req;
    std::vector<std::string>    asked_headers;
    std::vector<std::string>    allowed_headers;
    std::string                 sheaders;
    std::string                 s;
    std::string                 ress;
    size_t                      pos;
    size_t                      start_pos;

    req = client->request;
    sheaders = req->request_map["access-control-request-headers"];
    pos = sz(sheaders);
    allowed_headers = client->response->configs->
    while (pos == sz(sheaders) || pos != std::string::npos)
    {
        start_pos = pos == sz(sheaders) ? 0 : pos + 1;
        pos = sheaders.find(',', pos == sz(sheaders) ? 0 : pos + 1);
        s = pos == std::string::npos ? sheaders.substr(start_pos) : sheaders.substr(start_pos, pos - start_pos);
        s = get_upper_case(s);
        asked_headers.push_back(s);        
    }
    for (int i = 0; i < sz(asked_headers); i++)
    {
        if (std::find(allowed_methods.begin(), allowed_methods.end(), asked_headers[i]) != allowed_methods.end())
            ress = (sz(ress)) ? ress + ", " + asked_headers[i] : asked_headers[i];
    }
    return (ress) ;

}

void    Options::serve_client(t_client *client)
{
    t_request   *req;
    std::string ress;

    req = client->request;
    if (IN_MAP(req->request_map, "access-control-request-method"))
        ress += "access-control-allow-methods: " + get_allowed_methods(client) + "\r\n";
    if (IN_MAP(req->request_map, "access-control-request-headers"))
        ress += "access-control-allow-headers: " + get_allowed_headers(client) + "\r\n";
    if (IN_MAP(req->request_map, "access-control-max-age"))
        ress += "access-control-max-age: 0\r\n" ;
    if (IN_MAP(req->request_map, "access-control-credentials"))
        ress ++ "access-control"
}