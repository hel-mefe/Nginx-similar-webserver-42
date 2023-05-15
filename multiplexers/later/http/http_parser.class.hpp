#ifndef HTTP_PARSER_HPP
# define HTTP_PARSER_HPP

// # include "../structs/client.struct.hpp"
// # include "../structs/request.struct.hpp"
# include "../socket.hpp"

class HttpParser
{
    private:
        HashMap<std::string, std::string> *mimes;
        HashMap<int, std::string>         *codes;
    public:
        HttpParser(HashMap<std::string, std::string> *_mimes, HashMap<int, std::string> *_codes) : mimes(_mimes), codes(_codes){}
        ~HttpParser(){}

        void    set_mimes(HashMap<std::string, std::string> *_mimes){mimes = _mimes;}
        void    set_codes(HashMap<int, std::string> *_codes){codes = _codes;}

        bool                        is_request_uri_valid(std::string &path);
        std::string                 get_upper_case(std::string s);
        std::string                 get_lower_case(std::string s);
        std::string                 get_extension(std::string &s);
        std::vector<std::string>    *split_first_line(std::string &s);
        bool                        read_header(t_client *client);
        void                        parse_first_line(t_request *req);
        void                        parse_request(t_client *client);

} ;

#endif