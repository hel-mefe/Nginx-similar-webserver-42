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
    LOCATION,
    ERROR_PAGE
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

# define isReadingState(state) ((state == READING_HEADER || state == READING_BODY || state == SERVING_POST_REQUEST))
# endif