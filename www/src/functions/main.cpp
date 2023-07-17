# include "../includes/webserv.class.hpp"
# include "../includes/globals.hpp"

#define IS_MULTIPLEXER(m) (m == "kqueue" || m == "epoll" || m == "select" || m == "poll")

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
    cli_tokens.insert(std::make_pair("--multiplexer", MULTIP));
    cli_tokens.insert(std::make_pair("-m", MULTIP));
}

t_cli   *parse_and_get_cli(int ac, char **av)
{
    t_cli                       *c;
    HashMap<std::string, TOKEN> cli_tokens;
    TOKEN                       expecting;

    if (ac < 2)
        return (nullptr) ;
    init_cli_tokens(cli_tokens);
    c = new t_cli();
    expecting = NOTHING;
    for (int i = 2; i < ac; i++)
    {
        std::string arg = av[i];

        arg = get_lower_case(arg);
        if (expecting != NOTHING)
        {
            if (expecting == MULTIP && IS_MULTIPLEXER(arg))
                c->multiplexer = arg;
            else
            {
                delete c;
                return (nullptr) ;
            }
            expecting = NOTHING;
        }
        else if (!IN_MAP(cli_tokens, arg))
        {
            delete c;
            return (nullptr) ;
        }
        if (arg == "--activate-logs")
            c->is_logs_activated = true;
        if (arg == "--activate-strict-mode")
            c->is_strict_mode_activated = true;
        if (arg == "--help")
            c->is_help = true;
        if (arg == "-m" || arg == "--multiplexer")
            expecting = MULTIP ;
    }
    if (expecting != NOTHING)
    {
        delete c;
        return nullptr ;
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
            ws->set_cli(c);

            std::cout << LOGO << std::endl;
            if (c->is_help)
                std::cout << INTRO << README << std::endl;
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
