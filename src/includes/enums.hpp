#ifndef ENUMS_HPP
# define ENUMS_HPP

typedef struct server t_server;
typedef struct LocationConfigs t_location_configs;

#define MAX_FDS 256
#define MAX_BUFFER_SIZE 1000
#define MAX_CLIENTS 256
#define UNDEFINED_PORT 0
#define MAX_REQUEST_TIMEOUT 30

#define HTTP_VERSION "HTTP/1.1"
#define MAX_REQUEST_URI_SIZE 2048


# define sz(x) ((int)x.size())

#define SOCKET int
#define PORT short int

#define IN_MAP(m,e) (m.find(e) != m.end())

#define R_STATE(state) (state == READING_HEADER || state == SERVING_POST || state == SERVING_PUT)
#define W_STATE(state) (!(R_STATE(state)))

enum SOCKET_TYPE
{
    LISTENER,
    CLIENT
} ;

enum CLIENT_STATE
{
    WAITING,
    READING_HEADER,
    KEEP_ALIVE,
    SERVING_GET,
    SERVING_POST,
    SERVING_DELETE,
    SERVING_OPTIONS,
    SERVING_PUT,
    SERVING_TRACE,
    SERVING_CGI,
    SERVED,
} ;

enum MSG_TYPE
{
    INFO,
    WARNING,
    SUCCESS,
    ERROR,
    NORMAL,
} ;


#endif