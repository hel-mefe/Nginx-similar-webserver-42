#ifndef EXCEPTIONS_HPP
# define EXCEPTIONS_HPP

# include "header.hpp"

class PortAlreadyUsed : public std::exception
{
    public:
        const char *what() const _NOEXCEPT
        {
            return ("Port is already been used by another server");
        }
} ;

class InvalidMethod : public std::exception
{
    public:
        const char *what() const _NOEXCEPT
        {
            return ("Invalid method has been provided in the configuration file");
        }
} ;

class InvalidHttpToken : public std::exception
{
    public:
        const char *what() const _NOEXCEPT
        {
            return ("Invalid http token has been provided in the configuration file");
        }
} ;

class SocketException : public std::exception
{
    public:
        const char *what() const _NOEXCEPT
        {
            return ("Error occured during opening the socket descriptor");
        }
} ;

class BindException : public std::exception
{
    public:
        const char *what() const _NOEXCEPT
        {
            return ("Bind error");
        }
} ;

class ListenException : public std::exception
{
    public:
        const char *what() const _NOEXCEPT
        {
            return ("Listen error");
        }
} ;

class MultiplexerException : public std::exception
{
    public:
        const char *what() const _NOEXCEPT
        {
            return ("Multiplexer error!");
        }
} ;

class ParsingException : public std::exception
{
    public:
        const char *what() const _NOEXCEPT
        {
            return ("Parsing error!") ;
        }
} ;

class ParsingExceptionCgi
{
    public:
        const char *what() const _NOEXCEPT
        {
            return ("Parsing error: the CGI path provided is not valid!");
        }
} ;

class ParsingExceptionMaxConnectionsInterval
{
    public:
        const char *what() const _NOEXCEPT
        {
            return ("Parsing error: you provided a value for max_connections outside the interval allowed");
        }
} ;

class ParsingExceptionMaxBodySizeInterval
{
    public:
        const char *what() const _NOEXCEPT
        {
            return ("Parsing error: you provided a value for max_body_size outside the interval allowed");
        }
} ;

class UndefinedPort : public std::exception
{
    public:
        const char *what() const _NOEXCEPT
        {
            return ("Undefined Port Error.");
        }
} ;


# endif