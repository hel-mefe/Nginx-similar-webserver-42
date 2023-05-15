# include "delete.class.hpp"
# include "../http_handler.utils.hpp"

void    Delete::fill_response(t_client *client, int code, std::string status_line, bool write_it)
{
    t_request *req;
    t_response *res;
    std::map<std::string, std::string>  *request_map;
    std::string connection;
    CLIENT_STATE    new_state;

    req = client->request;
    res = client->response;
    request_map = &req->request_map;
    res->http_version = HTTP_VERSION;
    res->status_code = std::to_string(code);
    res->status_line = status_line ;
    connection = req->get_param("connection"); // used for keep-alive
    if (sz(connection)) // type of connection
        res->add("connection", "closed");
    if (write_it)
        res->write_response_in_socketfd(client->fd);
}

bool    Delete::location_has_cgi(t_client *client, std::string path)
{
    std::string ext = get_extension(path);

    return (IN_MAP(client->server->server_configs->extension_cgi, ext));
}

void    Delete::handle_delete_file(t_client *client)
{
    t_request                   *req;
    t_response                  *res;
    t_server_configs            *s_configs;
    t_location_configs          *l_configs;
    std::string                 path;

    std::cout << RED_BOLD << "DELETE SERVING IS RUNNING FOR FILE..." << WHITE << std::endl;
    req = client->request;
    res = client->response;
    s_configs = res->configs;
    l_configs = res->dir_configs;
    path = res->rootfilepath;
    if (location_has_cgi(client, path))
    {
        // call cgi to do the deletion job
        int k = 0;
    }
    else
    {
        path = std::string(getwd(NULL)) + res->rootfilepath;
        std::cout << YELLOW_BOLD << "FULL PATH OF FILE => " << path << WHITE << std::endl;
        if (!unlink(path.c_str())) // deleted succesfully
            fill_response(client, 204, "No Content", true);
        else
            fill_response(client, 403, "Forbidden", true);
    }
}

void    Delete::handle_delete_folder(t_client *client)
{
    t_request                   *req;
    t_response                  *res;
    t_server_configs            *s_configs;
    t_location_configs          *l_configs;
    std::string                 path;
    std::vector<std::string>    indexes;

    std::cout << RED_BOLD << "DELETE SERVING IS RUNNING FOR FOLDER..." << WHITE << std::endl;
    req = client->request;
    res = client->response;
    s_configs = res->configs;
    l_configs = res->dir_configs;
    path = res->rootfilepath;
    indexes = l_configs ? l_configs->indexes : s_configs->indexes;
    if (path[sz(path) - 1] != '/')
        fill_response(client, 409, "Conflict", true);
    else if (!set_file_path(path, indexes)) // if got passed this then there are indexes
        fill_response(client, 403, "Forbidden", true);
    else
    {
        std::cout << CYAN_BOLD << "FOLDER PATH => " << path << std::endl;
        if (location_has_cgi(client, path)) // run file with CGI
        {
            std::cout << YELLOW_BOLD << "LOCATION HAS CGI FOR DELETE!" << WHITE << std::endl;
        }
        else
        {
            path = std::string(getwd(NULL)) + res->rootfilepath;
            std::cout << RED_BOLD << "FOLDER TO BE DELETED IS => " << path << std::endl ;
            if (!unlink(path.c_str())) // success unlink returns 0 otherwise -1 and errno is set
                fill_response(client, 204, "No Content", true);
            else
            {
                if (access(path.c_str(), W_OK)) // does not have write access on folder
                    fill_response(client, 403, "Forbidden", true);
                else
                    fill_response(client, 500, "Internal Server Error", true);
            }
        }
    }
}


void    Delete::serve_client(t_client *client)
{
    if (client->request->is_file)
        handle_delete_file(client);
    else
        handle_delete_folder(client);
    client->state = SERVED;
}
