#pragma once

# include "header.hpp"
# include "exceptions.hpp"
# include "structs.hpp"
# include "tokenschecker.class.hpp"
# include "http_handler.utils.hpp"

#define DAYS_TO_SECS(ndays) (ndays * 24 * 60 * 60)
#define MINS_TO_SECS(nmins) (nmins * 60)
#define FROM_KB_TO_B(v) (v * 1000)
#define FROM_MB_TO_B(v) (v * 1000 * 1000)
#define FROM_GB_TO_B(v) (v * 1000 * 1000 * 1000)

typedef struct ast_node
{
    std::string                              id;
    std::vector<std::vector<std::string> >   words;
    std::vector<std::vector<std::string> >   location_blocks;
} t_node;

class ConfigFileParser
{
    private:
        TokensChecker                                                   tc; // TokensChecker
        std::vector<std::string>                                        lines;
        std::vector<std::vector<std::string> >                          words;
        std::vector<std::pair<std::string, std::pair<int, int> > >      brackets_q;
        std::vector<t_node>                                             nodes; // 1st one for http and others for servers
        std::vector<std::vector<std::string> >                          http_as_words;
        HashSet<int>                                                    ports_set;
        HashMap<std::string, std::string>                               http_configs;
        HashMap<std::string, TOKEN>                                     http_tokens; // token string mapped to the type of answer it accepts
        HashMap<std::string, TOKEN>                                     server_tokens; // same as above map
        HashMap<std::string, TOKEN>                                     location_tokens;

    public:
        ConfigFileParser();
        ConfigFileParser(const ConfigFileParser& p);
        ConfigFileParser& operator=(const ConfigFileParser& p) ;
        ~ConfigFileParser();

        std::string  get_config_file() const;
        bool    is_config_file_valid(std::string &config_file);
        // bool    parse_config_file(vector<Server *> &Servers, t_http_configs &globalConfigs);
        void    extract_lines_from_file(std::fstream &f_stream, std::vector<std::string> &lines);

        void    extract_words_from_lines(std::vector<std::vector<std::string> > &words);
        std::vector<std::string> get_word_vector(std::string &line);   
        void    build_brackets_queue();
        void    build_with_vector();

        std::string     get_identifier(std::pair<int, int> &start, std::pair<int, int> &end);
        void            fill_words(std::vector<std::vector<std::string> > &n_words, \
        std::pair<int, int> &start, std::pair<int, int> &end);
        void            push_node(std::pair<int, int> prev, std::pair<int, int> &mid, std::pair<int, int> &end);

        void            parse_words();
        void            parse_server(int &i, int &j);
        void            parse_server_location(t_node &node, int &i, int &j);
        void            parse_http_configs(int &i, int &j);

        void        fill_http_hashmap();
        void        fill_server_hashmap();
        void        fill_location_hashmap();
        void        fill_tokens();

        bool is_http_valid();
        bool is_http_line_valid(int row);
        bool is_servers_valid();
        bool is_server_line_valid(std::vector<std::string> &_words);
        bool parse_http();
        bool    is_location_block_valid(std::vector<std::string> &block);

        bool    fill_servers_data(std::vector<t_server *> *servers, t_http_configs *conf);
        bool    fill_http_data(t_http_configs *http_data);
        void    fill_server_attributes(t_server_configs &attr, t_http_configs *conf, int i);
        void    fill_location_attributes(std::string &cwd, t_server_configs *s_confs, t_location_configs &l_configs, int i, int j, int ej);
        void    handle_locations(t_server *server, std::vector<std::vector<std::string> > &location_blocks, t_server_configs *s_conf, int index);
        // data parsing getters;
        bool                        get_auto_indexing(std::vector<std::string> &line);
        bool                        get_connection(std::vector<std::string> &line);
        int                         get_port(std::vector<std::string> &line);
        unsigned long long          get_size(std::string &s);
        unsigned long long          get_time(std::string &s);
        std::vector<std::string>    get_vector_of_data(std::vector<std::string> &line); 
        HashSet<std::string>        vector_to_hashset(std::vector<std::string> &vec);
        void                        insert_cgi_to_hashmap(std::string cwd, HashMap<std::string, std::string> &extension_cgi, std::vector<std::string> &line);

        bool    parse_config_file(std::string config_file, t_http_configs *http_data, std::vector<t_server *> *servers);

        int count_words();

        void    normalize_directories_vector(std::vector<std::string> &vec);
        std::string get_directory(std::string &cwd, std::string &s);
        std::string get_location(std::string s);
} ;