# include "../includes/delete.class.hpp"
# include "../includes/http_handler.utils.hpp"
# include <sys/types.h>
# include <dirent.h>

Delete::Delete(void) {del_files_num = 0;}
Delete::~Delete(void) {}

bool Delete::rmfiles(const char* dirname)
{
    char stat = true;
    DIR* dirp = opendir(dirname);
    if (!dirp) return false;
    dirent* dp = readdir(dirp);
    while(dp != NULL)
    {
        std::string entname(dp->d_name);
        std::string path = dirname + entname;
        if(dp->d_type == DT_DIR)
        {
            if (entname != "." && entname != "..")
            {
                path.append("/");
                if(!rmfiles(path.c_str()))
                    stat = false;
            }
        }
        else
        {
            if(remove(path.c_str()))
                stat = false;
            else
                del_files_num++;
        }
        dp = readdir(dirp);
    }
    closedir(dirp);
    if (stat)
        remove(dirname);
    return stat;
}

void    Delete::fill_response(t_client *client, int code, std::string status_line, bool write_it)
{
    t_request *req;
    t_response *res;
    std::map<std::string, std::string>  *request_map;
    std::string connection;

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
        res->write_response_in_socketfd(client->fd, true);
}

void    Delete::handle_delete_file(t_client *client)
{
    t_response                  *res;
    std::string                 path;

    res = client->response;
    path = client->cwd + res->rootfilepath;
    if (access(path.c_str(), F_OK))
        fill_response(client, 404, "Not Found", true);
    else if (!remove(path.c_str())) // deleted succesfully
        fill_response(client, 204, "No Content", true);
    else
        fill_response(client, 403, "Forbidden", true);

}

void    Delete::handle_delete_folder(t_client *client)
{
    t_response  *res = client->response;
    std::string path = client->cwd + res->rootfilepath;

    if (access(path.c_str(), F_OK))
        fill_response(client, 404, "Not Found", true);
    else if (rmfiles(path.c_str()))
        fill_response(client, 204, "No Content", true);
    else
    {
        if (!del_files_num)
            fill_response(client, 403, "Forbidden", true);
        else
            fill_response(client, 409, "Conflict", true);
    }
    del_files_num = 0;
}


void    Delete::serve_client(t_client *client)
{
    if (client->request->is_file)
        handle_delete_file(client);
    else
        handle_delete_folder(client);
    client->state = SERVED;
}