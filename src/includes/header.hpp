#ifndef HEADER_HPP
# define HEADER_HPP

# include <iostream>
# include <vector>
# include <utility>
# include <list>
# include <unordered_map>
# include <unordered_set>
# include <queue>
# include <stack>
# include <map>
# include <set>
# include <fstream>
# include <ios>
# include <string>
# include <exception>

/*** Sockets dependencies ***/
# include <sys/socket.h>
# include <arpa/inet.h>
# include <sys/types.h>
# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <fcntl.h>
# include <netdb.h>
# include <sys/event.h>

/**** Default values for parsing data *****/
# define DEFAULT_CLIENT_MAX_BODY_SIZE 1000000
# define DEFAULT_CLIENT_MAX_REQUEST_TIMEOUT 45
# define DEFAULT_MAX_URI_SIZE 2048
# define DEFAULT_CGI_MAX_REQUEST_TIMEOUT 1000000
# define DEFAULT_KEEP_ALIVE_TIMEOUT 65
# define DEFAULT_CACHE_MAX_TIME (3 * 24 * 60 * 60) // 3 days
# define DEFAULT_CACHE_MAX_SIZE (12 * pow(1000, 3)) // 12mb
# define MAX_KEEP_ALIVE_TIMEOUT_ALLOWED 65
# define MAX_REQUEST_TIMEOUT_ALLOWED 45

# define MIME_TYPES_PATH "etc/mime.webserv"
# define HTTP_PATH "etc/http.webserv"

# define CONNECTIONS_POLL 2048
# define WAITING_TIMEOUT 10
# define READING_HEADER_TIMEOUT 10


# define PORT short int
# define SOCKET int
# define HashMap std::map
# define HashSet std::set

# define SA struct sockaddr
# define SA_IN struct sockaddr_in
# define SOCKET_NOT_DEFINED 0
# define UNDEFINED -1
# define ull unsigned long long
# define ll long long


# define ReadingEvent(change,fd,udata) EV_SET(change, fd, EVFILT_READ, EV_ADD, 0, 0, udata)
# define WritingEvent(change,fd,udata) EV_SET(change, fd, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, udata)
# define EofEvent(change,fd,udata) EV_SET(change, fd, EV_EOF, EV_ADD | EV_CLEAR, 0, 0, udata)
# define NoEvent(change,fd,udata) EV_SET(change, fd, EVFILT_USER | EV_EOF, EV_ADD | EV_CLEAR, 0, 0, udata)

typedef struct kevent k_event;

# define sz(x) ((int)x.size())
# define inHashMap(x,e) (x.find(e) != x.end())
# define inHashSet(x,e) (inHashMap(x,e))

#define IS_MULTIPLEXER(m) (m == "kqueue" || m == "epoll" || m == "select" || m == "poll")
#define IS_RESPONSE_HEADER_COMPLETE(client) ((client->request->method == "HEAD" || !sz(client->response->filepath)) && !client->response->is_directory_listing)
#define SUCCESS(syscall_expression) (syscall_expression != -1)
#define FAILURE(syscall_expression) (syscall_expression == -1)
// # define READ_FILTER EVFILT_READ
// # define WRITE_FILTER EVFILT_WRITE
// # define EOF_EVENT EV_EOF

enum MULTIPLEXER
{
    KQUEUE,
    POLL,
    SELECT,
    EPOLL
} ;

# define HEADER_PARSING 1
# define HEADER_CHECKING 2
# define RESPONSE_BUILDING 3

#ifdef __FreeBSD__
 # define MAIN_MULTIPLEXER KQUEUE
#endif

#ifdef __APPLE__
 # define MAIN_MULTIPLEXER KQUEUE
#endif

#ifdef __linux__
 # define MAIN_MULTIPLEXER EPOLL
#endif

enum TOKEN
{
    SHORT_INT,
    INT,
    DIRECTORY,
    STRING,
    STRING_VECTOR,
    METHOD,
    METHOD_VECTOR,
    ON_OFF,
    CONNECTION,
    CGI,
    LOCATION,
    ERROR_PAGE,
    MULTIP,
    SIZE,
    DATE,
    NOTHING
} ;

enum EVENT_TYPE
{
    READ,
    WRITE
} ;

# define isReadingState(state) ((state == READING_HEADER || state == READING_BODY || state == SERVING_POST_REQUEST))

#define INTRO WHITE_BOLD"Webserver42 is an Nginx like Webserver that was made for educational purposes by 3 1337 students, it is \n\
all about recoding Nginx from scratch with socket programming. of course, this webserver is meant for production use\n\
but it is 90% behaves the same way Nginx serves requests under HTTP/1.1 protocol.\n\
\n\
The following options are available for program cli: \n\
 - "GREEN_BOLD"-m [multiplexer]: "WHITE_BOLD"takes a multiplexer whether kqueue, epoll, poll or select and builds all the servers with it\n\
 - "GREEN_BOLD"--multiplexer [multiplexer]: "WHITE_BOLD"similar as above\n\
 - "GREEN_BOLD"--activate-strict_mode: "WHITE_BOLD"activates the strict mode, in this mode the program will not run unless there is no warnings\n\
 - "GREEN_BOLD"--activate-debugging_mode: "WHITE_BOLD"activates the debugging mode therefore the server administrator can debug requests and responses\n\
 - "GREEN_BOLD"--activate-caches: "WHITE_BOLD"activates the caching system to work, which mean any request that requires some processing will be served \n\
 - "GREEN_BOLD"--register-logs: "WHITE_BOLD"register all the logs in a file, the file can be accessed in the current directory as webserver.logs\n\
 from the cache in case the same asset was already requested before\n\
 - "GREEN_BOLD"--register-caches: "WHITE_BOLD"register all the requests that require some processing in the cache therefore the future requests can be\n\
 served fastly from the cache\n\n"


#define README "\
The following tokens are available for the http block: \n\
 - "GREEN_BOLD"root [string]: "WHITE_BOLD"defines the root directory\n\
 - "GREEN_BOLD"allowed_methods [METHOD1, METHOD2 ...]: "WHITE_BOLD" or * asterisk for all the methods, identifies the \n\
 http methods allowed in the whole webserver\n\
 - "GREEN_BOLD"client_max_body_size [number]:"WHITE_BOLD" identifies the max body size per request\n\
 - "GREEN_BOLD"client_max_request_timeout [number]:"WHITE_BOLD" identifies the max request time the webserver waits \n\
 for the request\n\
  - "GREEN_BOLD"cgi_max_request_timeout [number]: "WHITE_BOLD" specifies number of seconds to wait for the cgi process before it gets killed, \n\
the default value is 30 seconds\n\
  - "GREEN_BOLD"keep_alive_max_timeout [number]: "WHITE_BOLD" specifies the number of seconds to wait in a keep alive connection when the client has written nothing, \n\
the default value is 65 seconds\n\
 - "GREEN_BOLD"multiplexer [takes one of these 'kqueue' or 'epoll' or 'poll' or 'select']:"WHITE_BOLD" specifies the multiplexer used for simultaneous \n\
 connections, kqueue is the default one for FreeBSD and Apple distributions meanwhile Epoll is the default\n\
 one for Linux distributions\n\
 - "GREEN_BOLD"support_cookies [on/off]: "WHITE_BOLD" specifies if the webserver supports cookies or not\n\
 - "GREEN_BOLD"proxy_cache [on/off]: "WHITE_BOLD" specifies if the server should serve requests from the cache or not, the default is off\n\
 - "GREEN_BOLD"proxy_cache_register [on/off]: "WHITE_BOLD" specifies if the server should register the requests that require some processing for future use \
 the default is off\n\
 - "GREEN_BOLD"proxy_cache_max_time [time] ex. (10s, 10m, 10h, 10d): "WHITE_BOLD" specifies the period of time a request should get cached, 3 days is the default\n\
 - "GREEN_BOLD"proxy_cache_max_size [size] ex. (10by, 10kb, 10mb, 10gb): "WHITE_BOLD" specifies max size of the caches that the server should never surpass, \
 12mb is the default \n\n\
The following tokens are available for each server block: \n\
 - "GREEN_BOLD"server_name [string]:"WHITE_BOLD" specifies the server name\n\
 - "GREEN_BOLD"listen [number]:"WHITE_BOLD" specifies the port the virtual server should listen on\n\
 - "GREEN_BOLD"try_index_files [index1, index2, ... indexN]:"WHITE_BOLD" takes index files that should be served as indexes\n\
 in case a directory has been requested rather than a normal file\n\
 - "GREEN_BOLD"try_404_files [file1, file2, ... fileN]:"WHITE_BOLD" takes 404 files that should be\n\
 served in case the requested path was not found\n\
 - "GREEN_BOLD"allowed_methods [*] for all the methods or [METHOD1, METHOD2, ... METHOD_N]:"WHITE_BOLD" takes multiple methods that \n\
 should be supported by the server\n\
 - "GREEN_BOLD"root [string]:"WHITE_BOLD" takes the root directory and starts serving files starting from it\n\
 - "GREEN_BOLD"client_max_connections [number]:"WHITE_BOLD" takes the number of the maximum simultaneous connections that a server can handle\n\
 - "GREEN_BOLD"error_page [error_number] [error_file]:"WHITE_BOLD" takes the number of the error and whenever the server has to respond \n\
 with that http code it servers the provided file as http body response\n\
 - "GREEN_BOLD"cgi_bin [.extension] [/bin_path]:"WHITE_BOLD" takes the extension and maps it with the bin cgi path that should be run in case a file \n\
 with the provided extension has been provided\n\
 - "GREEN_BOLD"max_client_request_timeout:"WHITE_BOLD" maximum time the server should wait for the client to write something as request, \n\
 the default is 2 seconds in case not provided\n\
 - "GREEN_BOLD"max_client_body_size:"WHITE_BOLD" maximum body size the server should consider from the client as http request\n\
  - "GREEN_BOLD"cgi_max_request_timeout [number]: "WHITE_BOLD" specifies number of seconds to wait for the cgi process before it gets killed, \n\
the default value is 30 seconds\n\
  - "GREEN_BOLD"keep_alive_max_timeout [number]: "WHITE_BOLD" specifies the number of seconds to wait in a keep alive connection and the client has written nothing, \n\
the default value is 65 seconds\n\
 - "GREEN_BOLD"location [location]:"WHITE_BOLD" defines a location block inside the server block more details about it below\n\
\n\
The following tokens are available for each location block: \n\
 - "GREEN_BOLD"try_index_files [index1, index2, ... indexN]:"WHITE_BOLD" takes index files that should be served as indexes \n\
 in case this directory has been requested\n\
 - "GREEN_BOLD"try_404_files [file1, file2, ... fileN]:"WHITE_BOLD" takes 404 files that should be served in case the requested path was not found\n\
 - "GREEN_BOLD"redirect [path]:"WHITE_BOLD" takes a path and redirects the client to it whenever this location has been requested\n\
 - "GREEN_BOLD"support_cookies [on/off]:"WHITE_BOLD" specifies if the requested directory supports cookies or not\n\
 - "GREEN_BOLD"directory_listing [on/off]:"WHITE_BOLD" the default value depends on the server block, if specifies on then whenever there is no file \n\
 to serve and this location is requested the directory_listing will run\n\
"


# define RESET_CACHES_MSG YELLOW"Warning: "WHITE"Resetting the cache in our webserver is an irreversible action. Performing this operation will result\n\
 in the complete loss of all cached data, including temporarily stored content and frequently accessed resources. Any benefits gained from caching,\n\
 such as improved website performance and reduced load times, will be temporarily lost until the cache repopulates with fresh data.\n\n\
 Please confirm your understanding and acceptance of the risks involved by typing 'yes' if you wish to proceed with cache reset. Otherwise, type 'no' to reconsider \n\
 the operation. (yes/no)? "

 # define RESET_LOGS_MSG YELLOW"Warning:"WHITE" Clearing the logs in our system is an irreversible action. Performing this operation will result in the complete \n\
 deletion of all log data, including critical system logs, error logs, access logs, and other valuable information. Logs play a crucial role in monitoring system health, \n\
 diagnosing issues, and investigating security incidents.\n\n\
 Please confirm your understanding and acceptance of the risks involved by typing 'yes' if you wish to proceed with log clearing. \n\
 Otherwise, type 'no' to reconsider the operation. (yes/no) "

#endif