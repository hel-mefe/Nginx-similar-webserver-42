#ifndef DELETE_HPP
# define DELETE_HPP

# include "interface.methodhandler.hpp"
# include "socket.hpp"

class Delete : public MethodHandler
{
    public:
        Delete();
        ~Delete();
        void    serve_client(t_client *client);
        void    handle_delete_file(t_client *client);
        void    handle_delete_folder(t_client *client);
        bool    rmfiles(const char* dirname, int* del_files);
} ;

#endif