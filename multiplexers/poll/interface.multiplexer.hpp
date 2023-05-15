#ifndef MULTIPLEXER_HPP
# define MULTIPLEXER_HPP

class Multiplexer
{
    public:
        virtual void    multiplex() = 0;
        virtual         ~Multiplexer();
} ;

#endif