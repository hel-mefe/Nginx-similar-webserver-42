#ifndef TOKENS_CHECKER_HPP
# define TOKENS_CHECKER_HPP

# include "../includes/header.hpp"

class TokensChecker
{
    private:
    
    public:
        TokensChecker();
        TokensChecker(const TokensChecker &c);
        TokensChecker& operator=(const TokensChecker &c);
        ~TokensChecker();

        bool is_directory(std::string s);
        bool is_int(std::string s);
        bool is_short_int(std::string s);
        bool is_method(std::string s);
        bool is_on_off(std::string s);
        bool is_connection(std::string s);
        bool is_extension(std::string s);
} ;

#endif