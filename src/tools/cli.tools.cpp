# include "../includes/header.hpp"
# include "../includes/http_handler.utils.hpp"
# include <dirent.h>

void    reset_logs()
{
    std::string line;
    std::string msg;

    std::cout << RESET_LOGS_MSG ;
    while (!sz(line) && std::cin.good())
        std::getline(std::cin, line);
    std::cout << std::endl;
    if (line == "y" || line == "n" || line == "yes" || line == "no")
    {
        if (line == "y" || line == "yes")
        {
            unlink("etc/logs");
            int fd = open("etc/logs", O_CREAT, 0777);
            if (fd != -1)
            {
                close(fd) ;
                msg = "logs have been reset succesfully.";
                throw_msg(msg, false, SUCCESS);
            }
            else
            {
                msg = "logs have not been reset, unexpected error occured";
                throw_msg(msg, false, ERROR);
            }

        }
    }
    else
    {
        msg = "the logs have not been reset." ;
        throw_msg(msg, false, ERROR);
    }
    std::cout << std::endl;
}

void    reset_caches()
{
    std::string line;
    std::string msg;

    std::cout << RESET_CACHES_MSG ;
    while (!sz(line) && std::cin.good())
        std::getline(std::cin, line);
    std::cout << std::endl;
    if (line == "y" || line == "n" || line == "yes" || line == "no")
    {
        if (line == "y" || line == "yes")
        {
            unlink("etc/caches");
            int fd = open("etc/caches", O_CREAT, 0777);
            if (fd != -1)
            {
                close(fd) ;
                msg = "caches have been reset succesfully.";
                throw_msg(msg, false, SUCCESS);
            }
            else
            {
                msg = "caches have not been reset, unexpected error occured";
                throw_msg(msg, false, ERROR);
            }

        }
    }
    else
    {
        msg = "the caches have not been reset." ;
        throw_msg(msg, false, ERROR);
    }
    std::cout << std::endl;
}

void    init_cli_tokens(HashMap<std::string, TOKEN> &cli_tokens)
{
    cli_tokens.insert(std::make_pair("--activate-strict_mode", NOTHING));
    cli_tokens.insert(std::make_pair("--deactivate-strict_mode", NOTHING));
    cli_tokens.insert(std::make_pair("--activate-caches", NOTHING));
    cli_tokens.insert(std::make_pair("--deactivate-caches", NOTHING));
    cli_tokens.insert(std::make_pair("--activate-debugging_mode", NOTHING));
    cli_tokens.insert(std::make_pair("--deactivate-debugging_mode", NOTHING));
    cli_tokens.insert(std::make_pair("--register-caches", NOTHING));
    cli_tokens.insert(std::make_pair("--register-logs", NOTHING));
    cli_tokens.insert(std::make_pair("--reset-caches", NOTHING));
    cli_tokens.insert(std::make_pair("--reset-logs", NOTHING));
    cli_tokens.insert(std::make_pair("--help", NOTHING));
    cli_tokens.insert(std::make_pair("--multiplexer", MULTIP));
    cli_tokens.insert(std::make_pair("-m", MULTIP));
}

t_cli   *parse_and_get_cli(int ac, char **av)
{
    t_cli                       *c;
    HashMap<std::string, TOKEN> cli_tokens;
    std::string                 arg1;
    TOKEN                       expecting;

    if (ac < 2)
        return (nullptr) ;
    init_cli_tokens(cli_tokens);
    c = new t_cli();
    expecting = NOTHING;
    arg1 = av[1];
    if (arg1 == "--reset-caches")
        c->is_reset_caches = true ;
    if (arg1 == "--reset-logs")
        c->is_reset_logs = true ;
    if (arg1 == "--help")
        c->is_help = true ;
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
        if (arg == "--register-logs")
            c->is_logs_activated = true;
        if (arg == "--activate-strict_mode")
            c->is_strict_mode_activated = true;
        if (arg == "--help")
            c->is_help = true;
        if (arg == "--activate-caches")
            c->is_cache_activated = true ;
        if (arg == "--register-caches")
            c->is_cache_register_activated = true;
        if (arg == "--activate-debugging_mode")
            c->is_debugging_mode = true;
        if (arg == "-m" || arg == "--multiplexer")
            expecting = MULTIP ;
        if (arg == "--reset-logs")
            c->is_reset_logs = true ;
        if (arg == "--reset-caches")
            c->is_reset_caches = true ;
        if (arg == "--deactivate-caches")
            c->is_cache_activated = false;
        if (arg == "--deactivate-debugging_mode")
            c->is_debugging_mode = false;
        if (arg == "--deactivate-strict_mode")
            c->is_debugging_mode = false;
    }
    if (expecting != NOTHING)
    {
        delete c;
        return nullptr ;
    }
    return (c);
}

bool    is_project_structure_valid()
{
    DIR         *caches;
    DIR         *etc;
    DIR         *bin;
    bool        is_valid;
    std::string msg;

    caches = opendir("caches");
    etc = opendir("etc");
    bin = opendir("bin");
    is_valid = (caches && etc && bin && (!access("caches", W_OK) && !access("etc", W_OK)));
    if (is_valid)
    {
        if (!access("etc/logs", F_OK) && access("etc/logs", W_OK))
            is_valid = false ;
        else if (!access("caches/cacherc", F_OK) && (access("caches/cacherc", R_OK) || access("caches/cacherc", W_OK)))
            is_valid = false ;
        else
        {
            if (access("etc/logs", F_OK))
            {
                int fd = open("etc/logs", O_CREAT, 0777);
                if (fd < 0)
                {
                    msg = "internal server error, open failed.";
                    throw_msg(msg, true, ERROR);
                }
                close(fd);
            }

            if (access("caches/cacherc", F_OK))
            {
                int fd = open("caches/cacherc", O_CREAT, 0777);
                if (fd < 0)
                {
                    msg = "internal server error, open failed.";
                    throw_msg(msg, true, ERROR);
                }
                close(fd);
            }

            if (access("caches/cachetm", F_OK))
            {
                int fd = open("caches/cachetm", O_CREAT, 0777);
                if (fd < 0)
                {
                    msg = "internal server error, open failed.";
                    throw_msg(msg, true, ERROR);
                }
                close(fd);
            }
        }
    }
    if (caches)
        closedir(caches);
    if (etc)
        closedir(etc);
    if (bin)
        closedir(bin);
    return (is_valid) ;
}

void    check_is_project_well_structured()
{
    std::string msg;

    if (!is_project_structure_valid())
    {
        msg = "project structure is not valid";
        throw_msg(msg, true, ERROR);
    }
}

void    help()
{
    std::cout << README << std::endl;
    exit(0);
}
