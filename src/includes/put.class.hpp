#pragma once

# include "interface.methodhandler.hpp"
# include "http_handler.class.hpp"
# include "socket.hpp"
# include <fstream>
#include <list>

class Put : public MethodHandler
{
    public:
        Put();
        void    serve_client(t_client*);
        ~Put();
} ;