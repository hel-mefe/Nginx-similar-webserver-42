#ifndef HTTP_HANDLER_HPP
# define HTTP_HANDLER_HPP

# include "socket.hpp"
# include "structs.hpp"
# include "http_parser.class.hpp"
# include "http_handler.utils.hpp"
# include "client.struct.hpp"
# include "class.handlers.hpp"

typedef struct request t_request;

class HttpHandler : public HttpHandlerInterface
{
    private:
        std::map<std::string, std::string> *mimes;
        std::map<int, std::string>         *codes;
        HttpParser                         *http_parser;
        Handlers                           *handlers;

    public:
        HttpHandler()
        {
            http_parser = new HttpParser();
            handlers = new Handlers(mimes, codes);
        }
        HttpHandler(std::map<std::string, std::string> *_mimes, std::map<int, std::string> *_codes)
        {
            http_parser = new HttpParser();
            mimes = _mimes;
            codes = _codes;
            handlers = new Handlers(mimes, codes);
        }
        ~HttpHandler(){
            delete http_parser;
            delete handlers;
        };

        void    set_mimes(std::map<std::string, std::string> *_mimes)
        {
            mimes = _mimes;
            handlers->set_mimes(_mimes);
        }

        void    set_codes(std::map<int, std::string> *_codes)
        {
            codes = _codes;
            handlers->set_codes(_codes);
        }


        void    handle_http(t_client *client);
        void    parse_request(t_client *client);
        void    architect_response(t_client *client);

        bool    is_method_valid(std::string &method);
        void    fill_response(t_client *client, int code, bool write_it);

        bool    set_redirection_path(t_client *client);
        void    set_configurations(t_client *client);
        void    set_response_configs(t_client *client);

        void    architect_post_response(t_client *client);

        bool    set_directory_indexes(t_client *client);

        t_location_configs  *get_location_configs_from_path(t_client *client);
        std::string get_longest_directory_prefix(t_client *client, std::string path, bool is_config);

        void        set_root_file_path(t_client *client);
        std::string get_root_file_path(std::string &root, std::string &path, std::string &dirpath, bool sfile);
        std::string get_clean_file_path(std::string root, std::string path, std::string dirpath, bool zfile);
        void        parse_first_line(t_request *req);
} ;

#endif