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





# endif