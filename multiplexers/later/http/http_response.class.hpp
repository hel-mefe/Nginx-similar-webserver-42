#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

# include "../socket.hpp"
// # include <unordered_map>

// #define HashMap std::unordered_map

class HttpResponse
{
    private:
        HashMap<std::string, std::string>   *mimes;
        HashMap<int, std::string>           *codes;
    public:
        HttpResponse(HashMap<std::string, std::string> *_mimes, HashMap<int, std::string> *_codes) : mimes(_mimes), codes(_codes){}
        ~HttpResponse(){}
} ;

#endif