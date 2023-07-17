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
    if (ac == 3 && std::string(av[2]) == "--help")
    {
        std::cout << LOGO << std::endl;
        std::cout << WHITE_BOLD << README << std::endl;
    }
    else if (ac != 2)
    {
        std::cout << LOGO << std::endl;
        std::cout << WHITE_BOLD << "please run webserv with the required arguments ./webserv [configfile]" << std::endl; 
        return (1) ; 
    }
    else
    {
        try
        {
            Webserver *ws = new Webserver(av[1]);

            std::cout << LOGO << std::endl;
            if (ws->parse_config_file())
            {
                // std::string ws->getWarnings();
                ws->run();
            }
        }
        catch (const std::exception &e) 
        {
            std::cout << e.what() << std::endl;
        }
    }
}
