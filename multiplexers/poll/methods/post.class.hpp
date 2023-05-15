#ifndef POST_HPP
# define POST_HPP

# include "interface.methodhandler.hpp"

class Post : public MethodHandler
{
    public:
        Post(){}
        void    serve_client(t_client *client)
        {
            std::cout << "Post METHOD IS BEING SERVED NOW ...." << std::endl;
        }
        ~Post(){}
} ;

#endif