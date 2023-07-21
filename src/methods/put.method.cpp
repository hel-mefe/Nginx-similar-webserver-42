# include "../includes/put.class.hpp"
# include <unistd.h>

Put::Put(void) {}
Put::~Put(void) {}

void Put::serve_client(t_client* client)
{
    t_request* req = client->request;
    
    if (req->first_time)
    {
        if (!req->is_file)
        {
            fill_response(client, 403, "Forbidden", true);
            client->state = SERVED;
            return;
        }
        if (!create_file(client))
            return;
        req->first_time = false;
    }
    upload_file(client);
}
