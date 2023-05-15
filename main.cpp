# include "webserver.class.hpp"
# include "multiplexers/poll/poll.class.hpp"

int main(int ac, char **av)
{
    if (ac != 2)
    {
        std::cout << "please provide arguments" << std::endl;
        return (1) ;
    }
    MultiplexerInterface *multiplexer = new Poll();
    Webserver *w = new Webserver(av[1], multiplexer);

    try
    {
        int k = open(av[1], O_RDONLY);
        char buff[1000];
        bzero(buff, 1000);
        while (read(k, buff, 1000) > 0)
            std::cout << buff << std::endl;
        if (w->parse_config_file())
        {
            std::cout << "-- YES VALID --" << std::endl;
            w->run();
        }
        else
            std::cout << RED_BOLD << "NOT VALID" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }
}