# include "../includes/webserv.class.hpp"
# include "../includes/globals.hpp"
# include "../includes/http_handler.utils.hpp"

#define IS_MULTIPLEXER(m) (m == "kqueue" || m == "epoll" || m == "select" || m == "poll")

#define LOGO GREEN_BOLD"\n\n██╗    ██╗███████╗██████╗ ███████╗███████╗██████╗ ██╗   ██╗███████╗██████╗       ██╗  ██╗██████╗ \n\
██║    ██║██╔════╝██╔══██╗██╔════╝██╔════╝██╔══██╗██║   ██║██╔════╝██╔══██╗      ██║  ██║╚════██╗ \n\
██║ █╗ ██║█████╗  ██████╔╝███████╗█████╗  ██████╔╝██║   ██║█████╗  ██████╔╝█████╗███████║ █████╔╝ \n\
██║███╗██║██╔══╝  ██╔══██╗╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝██╔══╝  ██╔══██╗╚════╝╚════██║██╔═══╝  \n\
╚███╔███╔╝███████╗██████╔╝███████║███████╗██║  ██║ ╚████╔╝ ███████╗██║  ██║           ██║███████╗ \n\
 ╚══╝╚══╝ ╚══════╝╚═════╝ ╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝  ╚══════╝╚═╝  ╚═╝           ╚═╝╚══════╝ \n\n"

void    print_warnings(std::vector<std::string> &warnings)
{
    for (int i = 0; i < sz(warnings); i++)
    {
        std::string s = warnings[i];

        std::cerr << YELLOW_BOLD << "[WARNING]: " << s << std::endl;
    }
    std::cout << WHITE_BOLD << std::endl;
}

int main(int ac, char **av)
{
    HashSet<std::string>    cli_tokens;
    std::string             msg;
    t_cli     *c = parse_and_get_cli(ac, av);

    std::cout << LOGO << std::endl;
    if (!c)
    {
        msg = "please run webserv42 with the required arguments ./webserv [configfile] [cli_arguments]" ;
        throw_msg(msg, 1, INFO);
        return (1) ; 
    }

    if (c->is_help)
        help();
    check_is_project_well_structured();
    if (c->is_reset_logs)
    {
        reset_logs(true);
        if (ac == 2)
            return (0) ;
    }
    try
    {
        Webserver *ws = new Webserver(av[1]);
        ws->set_cli(c);

        if (ws->parse_config_file())
        {
            std::cout << GREEN_BOLD << "CONFIG FILE IS VALID" << std::endl;
            ws->print_all_data(); 
            // if (sz(warnings) && c->is_strict_mode_activated)
            //     return (1) ;
            ws->run();
        }
        else
        {
            msg = "The provided config file [" + std::string(av[1]) + "] is invalid.";
            throw_msg(msg, true, ERROR);
        }
    }
    catch (const std::exception &e) 
    {
        std::cout << e.what() << std::endl;
    }
}
