# include "../includes/webserv.class.hpp"
# include "../includes/globals.hpp"

#define LOGO GREEN_BOLD"\n\n██╗    ██╗███████╗██████╗ ███████╗███████╗██████╗ ██╗   ██╗███████╗██████╗       ██╗  ██╗██████╗ \n\
██║    ██║██╔════╝██╔══██╗██╔════╝██╔════╝██╔══██╗██║   ██║██╔════╝██╔══██╗      ██║  ██║╚════██╗ \n\
██║ █╗ ██║█████╗  ██████╔╝███████╗█████╗  ██████╔╝██║   ██║█████╗  ██████╔╝█████╗███████║ █████╔╝ \n\
██║███╗██║██╔══╝  ██╔══██╗╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝██╔══╝  ██╔══██╗╚════╝╚════██║██╔═══╝  \n\
╚███╔███╔╝███████╗██████╔╝███████║███████╗██║  ██║ ╚████╔╝ ███████╗██║  ██║           ██║███████╗ \n\
 ╚══╝╚══╝ ╚══════╝╚═════╝ ╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝  ╚══════╝╚═╝  ╚═╝           ╚═╝╚══════╝ \n\n"


int main(int ac, char **av)
{
    if (ac != 2)
    {
        std::cout << LOGO << std::endl;
        std::cout << WHITE_BOLD << "Please run webserv with the required arguments ./webserv [configfile]" << std::endl; 
        return (1) ; 
    }
    try
    {
        Webserver *ws = new Webserver(av[1]);

        char buff[1000];
        bzero(buff, 1000);
        std::cout << LOGO << std::endl;
        if (ws->parse_config_file())
            ws->run();
    }
    catch (const std::exception &e) 
    {
        std::cout << e.what() << std::endl;
    }
}
