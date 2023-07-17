# include "../includes/webserv.class.hpp"
# include "../includes/globals.hpp"

#define LOGO GREEN_BOLD"\n\n██╗    ██╗███████╗██████╗ ███████╗███████╗██████╗ ██╗   ██╗███████╗██████╗       ██╗  ██╗██████╗ \n\
██║    ██║██╔════╝██╔══██╗██╔════╝██╔════╝██╔══██╗██║   ██║██╔════╝██╔══██╗      ██║  ██║╚════██╗ \n\
██║ █╗ ██║█████╗  ██████╔╝███████╗█████╗  ██████╔╝██║   ██║█████╗  ██████╔╝█████╗███████║ █████╔╝ \n\
██║███╗██║██╔══╝  ██╔══██╗╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝██╔══╝  ██╔══██╗╚════╝╚════██║██╔═══╝  \n\
╚███╔███╔╝███████╗██████╔╝███████║███████╗██║  ██║ ╚████╔╝ ███████╗██║  ██║           ██║███████╗ \n\
 ╚══╝╚══╝ ╚══════╝╚═════╝ ╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝  ╚══════╝╚═╝  ╚═╝           ╚═╝╚══════╝ \n\n"

void    init_cli_tokens(HashMap<std::string, TOKEN> &cli_tokens)
{
    cli_tokens.insert(std::make_pair("--activate-logs", NOTHING));
    cli_tokens.insert(std::make_pair("--activate-strict-mode", NOTHING));
    cli_tokens.insert(std::make_pair("--help", NOTHING));
}

t_cli   *parse_and_get_cli(int ac, char **av)
{
    t_cli                       *c;
    HashMap<std::string, TOKEN> cli_tokens;

    if (ac < 2)
        return (nullptr) ;
    init_cli_tokens(cli_tokens);
    c = new t_cli();
    for (int i = 2; i < ac; i++)
    {
        std::string arg = av[i];

        if (!IN_MAP(cli_tokens, arg))
        {
            delete c;
            return (nullptr) ;
        }
        c->is_logs_activated = (arg == "--activate-logs") ;
        c->is_strict_mode_activated = (arg == "--activate-strict-mode");
        c->is_help = (arg == "--help");
    }
    return (c);
}

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
    t_cli     *c = parse_and_get_cli(ac, av);

    if (!c)
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
            if (c->is_help)
                std::cout << README << std::endl;
            else
            {
                if (ws->parse_config_file())
                {
                    std::vector<std::string> warnings = ws->generate_all_warnings();
                    print_warnings(warnings);
                    if (sz(warnings) && c->is_strict_mode_activated)
                        return (1) ;
                    ws->run();
                }
            }
        }
        catch (const std::exception &e) 
        {
            std::cout << e.what() << std::endl;
        }
    }
}
