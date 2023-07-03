#ifndef DELETE_HPP
# define DELETE_HPP

# include "interface.methodhandler.hpp"
# include "socket.hpp"

class Delete : public MethodHandler
{
    private:
        int del_files_num;
    public:
        Delete();
        ~Delete();
        void    serve_client(t_client *client);
        void    handle_delete_file(t_client *client);
        void    handle_delete_folder(t_client *client);
        void    fill_response(t_client *client, int code, std::string status_line, bool write_it);
        bool    rmfiles(const char* dirname);
} ;

#endif