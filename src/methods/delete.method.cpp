# include "../includes/delete.class.hpp"
# include "../includes/http_handler.utils.hpp"
# include <sys/types.h>
# include <dirent.h>

Delete::Delete(void) {}
Delete::~Delete(void) {}

bool Delete::rmfiles(const char* dirname, int* del_files)
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
                if(!rmfiles(path.c_str(), del_files))
                    stat = false;
            }
        }
        else
        {
            if(remove(path.c_str()))
                stat = false;
            else
                (*del_files)++;
        }
        dp = readdir(dirp);
    }
    closedir(dirp);
    if (stat)
        remove(dirname);
    return stat;
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
    else if (rmfiles(path.c_str(), &res->del_files))
        fill_response(client, 204, "No Content", true);
    else
    {
        if (!res->del_files)
            fill_response(client, 403, "Forbidden", true);
        else
            fill_response(client, 409, "Conflict", true);
    }
    res->del_files = 0;
}


void    Delete::serve_client(t_client *client)
{
    if (client->request->is_file)
        handle_delete_file(client);
    else
        handle_delete_folder(client);
    client->state = SERVED;
}