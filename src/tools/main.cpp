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
    cli_tokens.insert(std::make_pair("--activate-caches", NOTHING));
    cli_tokens.insert(std::make_pair("--register-caches", NOTHING));
    cli_tokens.insert(std::make_pair("--activate-debugging-mode", NOTHING));
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
        if (arg == "--activate-caches")
            c->is_cache_activated = true ;
        if (arg == "--register-caches")
            c->is_cache_register_activated = true;
        if (arg == "--activate-debugging-mode")
            c->is_debugging_mode = true;
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

bool    is_project_structure_valid()
{
    DIR     *caches;
    DIR     *etc;
    DIR     *bin;
    bool    is_valid;

    caches = opendir("caches");
    etc = opendir("etc");
    bin = opendir("bin");
    is_valid = (caches && etc && bin && (!access("caches", W_OK) && !access("etc", W_OK)));
    closedir(caches);
    closedir(etc);
    closedir(bin);
    return (is_valid) ;
}

int main(int ac, char **av)
{
    HashSet<std::string>    cli_tokens;
    t_cli     *c = parse_and_get_cli(ac, av);

    std::cout << LOGO << std::endl;
    if (ac == 2 && std::string(av[1]) == "--help")
    {
        std::cout << INTRO << README << std::endl;
        return (EXIT_SUCCESS) ;
    }
    if (!is_project_structure_valid())
    {
        std::cerr << "[Webserv42]: project is not well structured, ./webserv --help to know more about the structure" << std::endl;
        return (EXIT_FAILURE) ;
    }
    if (!c)
    {
        std::cout << LOGO << std::endl;
        std::cout << WHITE_BOLD << "please run webserv with the required arguments ./webserv [configfile]" << std::endl; 
        return (EXIT_FAILURE) ; 
    }
    else
    {
        try
        {
            Webserver *ws = new Webserver(av[1]);
            ws->set_cli(c);

            if (c->is_help)
            {
                std::cout << INTRO << README << std::endl;
                return (EXIT_SUCCESS) ;
            }
            else
            {
                if (ws->parse_config_file())
                {
                    std::cout << GREEN_BOLD << "CONFIG FILE IS VALID" << std::endl;
                    ws->print_all_data(); 
                    std::vector<std::string> warnings = ws->generate_all_warnings();
                    print_warnings(warnings);
                    if (sz(warnings) && c->is_strict_mode_activated)
                        return (EXIT_FAILURE) ;
                    ws->run();
                }
                else
                    std::cout << RED_BOLD << "CONFIG FILE IS NOT VALID" << std::endl;
            }
        }
        catch (const std::exception &e) 
        {
            std::cout << e.what() << std::endl;
        }
    }
    return (EXIT_SUCCESS);
}
