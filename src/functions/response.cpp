# include "../includes/socket.hpp"

void    architect_response(t_client *client)
{
    t_request                           *req;
    t_response                          *res;
    std::map<std::string, std::string>  *req_map;

    req = client->request;
    res = client->response;
    req_map = &req->request_map;
    res->write_response_in_socketfd(client->fd, true);
}