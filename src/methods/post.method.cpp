#include "../includes/post.class.hpp"
#include <sys/time.h>

Post::Post(void) {}

void Post::serve_client(t_client *client)
{
    t_request* req = client->request;

    if (req->first_time)
    {
        req->body_size = 0;
        if (!create_file(client))
            return;
        req->first_time = false;
    }
    upload_file(client);
}

Post::~Post(void) {}