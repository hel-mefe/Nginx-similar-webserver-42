#ifndef HANDLERS_HPP
# define HANDLERS_HPP

# include "socket.hpp"
# include "structs.hpp"
# include "http_parser.class.hpp"
# include "http_handler.utils.hpp"
# include "client.struct.hpp"
# include "enums.hpp"


class Handlers
{
    private:
        std::map<std::string, std::string>  *mimes;
        std::map<int, std::string>          *codes;
    public:
        Handlers(std::map<std::string, std::string> *_mimes, std::map<int, std::string> *_codes)
        {
            mimes = _mimes;
            codes = _codes;
        }

        void    set_mimes(std::map<std::string, std::string> *_mimes)
        {
            mimes = _mimes;
        }

        void    set_codes(std::map<int, std::string> *_codes)
        {
            codes = _codes;
        }
    
        bool    handle_400(t_client *client);
        bool    handle_414(t_client *client);
        bool    handle_404(t_client *client);
        bool    handle_301(t_client *client);
        bool    handle_405(t_client *client);
        bool    handle_413(t_client *client);
        bool    handle_501(t_client *client);
        bool    handle_200(t_client *client);
        bool    handle_200f(t_client *client);
        bool    handle_200d(t_client *client);

        bool    change_path(t_client *client);
        bool    set_path_for_file(std::string &root, std::string &file);
        std::string get_path_after_change(t_client *client, std::string root);

        void    fill_response(t_client *client, int code, bool write_it);


} ;

#endif

/***
 * 
 * 
 * 
 *  /dir1/dir2/dir3 |  s_configs root /www | d_configs root /dir1/dir2 -> /ddd
 * /ddd/d
 * 
 * 
 * in case of errors I always have to work with root + file
 * 
 * in case of 200 I always have to work with changing the path to check if the file is exist
 * under this step there is 2 cases, handling a file and handling a directory
 * for file just check if exist or not
 * for directory set indexes if sucessful then done, otherwise set 404 files if none of these is sucessful then serve just the header
 * client state after that depends on the setting of indexes and 404 files, if successful the SERVING GET is the new state
 * if exist then I have to serve it, otherwise always start from the root  
 *  
*/