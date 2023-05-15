#ifndef NOT_NEEDED_HPP
# define NOT_NEEDED_HPP

# include "header.hpp"

typedef struct response
{
    std::string connection;
    std::string http_version;
    std::string status_line;
    std::string content_type;
    std::string full_path;
    std::string requested_path;
    std::string simplyfied_path;
    std::string file;
    std::string transfer_encoding;
    std::string location; // in 301 status always include this in the response
    std::string path;
    SOCKET      sockfd; // where to write
    int         content_length;
    int         status_code;
    bool        is_response_complete;
    int         fd;

    response() : content_length(0)
    {
        connection = "Close";
        http_version = "HTTP/1.1";
        transfer_encoding = "Chunked";
        status_code = 200;
        is_response_complete = false;
        fd = UNDEFINED;
    }
    ~response(){std::cout << "DESTRUCTOR WAS CALLED!" << std::endl;}
}   t_response ;

typedef struct Header
{
    std::vector<std::string>                        header_lines; // all the lines of the header stored in a  
    HashMap<std::string, std::string>               header_map; // header map (currently working with it)
    HashMap<std::string, int>                       header_map_ints; // header map for elements that accept ints (for future use)
    HashMap<std::string, std::vector<std::string> > header_map_vec; // header map for elements that accept multiple values (for future use)
    std::string                                     method; // by convention uppercased (POST, GET, DELETE)
    std::string                                     path; // self-explained (:D)
    std::string                                     http_version; // by convention uppercased (HTTP/1.1)
    std::string                                     connection; // for future use
    std::string                                     body; // for future use
    std::string                                     accept_content; // for future use
    std::string                                     header;
    std::string                                     extension; // the extension of the resource we have to serve
    std::string                                     file;
    int                                             content_length; // for future use
    int                                             request_len; // for future use
    int                                             body_length;
    bool                                            is_header_complete;
    bool                                            is_request_valid;
    bool                                            is_served;
    bool                                            has_write_socket;
    bool                                            is_directory;
    Header()
    {
        is_request_valid = true;
        is_directory = false;
        connection = "Close";
        http_version = "HTTP/1.1";
        body_length = 0;
    }

    ~Header(){}
}   t_request;

typedef struct http
{
    SOCKET          fd;
    t_request       *request;
    t_response      *response;
    SOCKET_STATE    state; // (READING_HEADER, READING_BODY, SERVING_REQUEST)
    time_t          request_time; // saving the time the request has been requested
    http() : request(new t_request()), response(nullptr), state(WAITING){}
    http(SOCKET _fd) : fd(_fd), request(new t_request()), response(new t_response()), state(WAITING){}
    ~http()
    {
        delete request;
        request = nullptr;
        delete response;
        response = nullptr;
    }
} t_http ;

typedef struct sock
{
    SOCKET          fd;
    SA_IN           data;
    socklen_t       data_len;
    SOCKET_TYPE     type; // (LISTENER, CLIENT_READ, CLIENT_WRITE)
    t_http          *http;
    sock() : fd(UNDEFINED), data_len(sizeof(data)), type(NONE)
    {
        bzero(&data, sizeof(data));
    }

    ~sock()
    {

    }
}   t_socket;

typedef struct globalVars
{
    HashSet<std::string>        supported_methods_set;
    HashMap<int, std::string>   codes_map;
}   globals;

typedef struct network
{
    int                             kq;
    unsigned int                    num_connections;
    k_event                         *changeList;
    k_event                         *eventList;
    HashMap<SOCKET, int>            socket_index;
    HashMap<int, int>               read_write;
    HashMap <SOCKET, t_http *>      socket_http;
    std::queue<int>                 clients_indexes;
    std::queue<int>                 waiting_connections;
    HashMap<SOCKET, k_event *>      connected_clients; // socket to appropriate k_event struct
}   t_network;

#endif