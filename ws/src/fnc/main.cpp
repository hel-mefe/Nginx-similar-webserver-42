# include "../inc/webServClass.hpp"

int main(int ac, char **av)
{
    if (ac != 2)
    { std::cout << "please provide arguments" << std::endl; return (1) ; }
    try
    {
        int conf = open(av[1], O_RDONLY);
        MultiplexerInterface *multiplexer = new Poll();
        Webserver *ws = new Webserver(av[1], multiplexer);
        char buff[1000];
        bzero(buff, 1000);
        while (read(conf, buff, 1000) > 0)
            std::cout << buff << std::endl;
        if (ws->parse_config_file())
            ws->run();
        else
            std::cout << RED_BOLD << "Config file is not valid!" << std::endl;
    }
    catch (const std::exception &e) {std::cout << e.what() << std::endl;}
}
