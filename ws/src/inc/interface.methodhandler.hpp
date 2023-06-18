#ifndef METHOD_HANDLER
# define METHOD_HANDLER

# include <iostream>

typedef struct client t_client;

class MethodHandler
{
    public:
        virtual void serve_client(t_client *client) = 0;
        virtual ~MethodHandler(){};
} ;

#endif