#ifndef DELETE_HPP
# define DELETE_HPP

# include "interface.methodhandler.hpp"
# include "../socket.hpp"

class Delete : public MethodHandler
{
    public:
        Delete(){}

        void    serve_client(t_client *client);
        void    handle_delete_file(t_client *client);
        void    handle_delete_folder(t_client *client);

        void    fill_response(t_client *client, int code, std::string status_line, bool write_it);
        bool    location_has_cgi(t_client *client, std::string path);
        ~Delete(){}
} ;

#endif