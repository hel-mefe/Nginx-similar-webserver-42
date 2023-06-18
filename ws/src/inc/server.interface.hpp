#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>

class ServerInterface
{
    public:
        virtual void    run_forked() = 0;
        virtual void    run_multiplexed() = 0;
        virtual ~ServerInterface();
} ;

#endif