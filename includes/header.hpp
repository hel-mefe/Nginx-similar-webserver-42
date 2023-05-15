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

// # define cout std::cout
// # define cin std::cin
// # define fstream std::fstream
// # define ifstream std::ifstream
// # define string std::string
// # define vector std::vector
// # define list std::list
// # define map std::map
// # define set std::set
// # define unordered_map std::unordered_map
// # define unordered_set std::unordered_set

# define MIME_TYPES_PATH "etc/mime.webserv"
# define HTTP_PATH "etc/http.webserv"

# define CONNECTIONS_POLL 2048
# define WAITING_TIMEOUT 10
# define READING_HEADER_TIMEOUT 10

# define PORT short int
# define SOCKET int
# define HashMap std::map
# define HashSet std::unordered_set

# define SA struct sockaddr
# define SA_IN struct sockaddr_in
# define SOCKET_NOT_DEFINED 0
# define UNDEFINED -1

# define ReadingEvent(change,fd,udata) EV_SET(change, fd, EVFILT_READ, EV_ADD, 0, 0, udata)
# define WritingEvent(change,fd,udata) EV_SET(change, fd, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, udata)
# define EofEvent(change,fd,udata) EV_SET(change, fd, EV_EOF, EV_ADD | EV_CLEAR, 0, 0, udata)
# define NoEvent(change,fd,udata) EV_SET(change, fd, EVFILT_USER | EV_EOF, EV_ADD | EV_CLEAR, 0, 0, udata)

typedef struct kevent k_event;

# define sz(x) ((int)x.size())
# define inHashMap(x,e) (x.find(e) != x.end())
# define inHashSet(x,e) (inHashMap(x,e))

// # define READ_FILTER EVFILT_READ
// # define WRITE_FILTER EVFILT_WRITE
// # define EOF_EVENT EV_EOF

# define HEADER_PARSING 1
# define HEADER_CHECKING 2
# define RESPONSE_BUILDING 3

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
    LOCATION
} ;

enum MULTIPLEXER
{
    KQUEUE,
    POLL,
    SELECT,
    EPOLL
} ;

enum EVENT_TYPE
{
    READ,
    WRITE
} ;

// enum SOCKET_TYPE
// {
//     LISTENER,
//     CLIENT_READ,
//     CLIENT_WRITE,
//     NONE
// } ;

// enum SOCKET_STATE
// {
//     READING_HEADER,// currently reading the header when it all reads needs to get parsed
//     CHECKING_HEADER, // checking header errors
//     READING_BODY,// currently reading the body part
//     BUILDING_GET_HEADER,
//     BUILDING_POST_HEADER,
//     BUILDING_DELETE_HEADER,
//     SERVING_GET_REQUEST,// request has GET method and the method handler should handle it
//     SERVING_POST_REQUEST,// request has POST method and the method handler should handle it
//     SERVING_POST_REQUEST_DONE,
//     SERVING_DELETE_REQUEST,// request has DELETE method and the method handler should handle it
//     SERVED,// request has been served succesfully!
//     WAITING// waiting for its turn (DEFAULT VALUE)
// } ;

# define isReadingState(state) ((state == READING_HEADER || state == READING_BODY || state == SERVING_POST_REQUEST))

// std::string PARSING_ERROR;
# endif