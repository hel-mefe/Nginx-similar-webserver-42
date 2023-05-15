#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <iostream>
# include <unistd.h>
# include <poll.h>
# include <vector>
# include <queue>
# include <list>
# include <map>
# include <unordered_map>
# include <set>
# include <deque>
# include <string>
# include <sstream>
# include <sys/socket.h>
# include <sys/fcntl.h>
# include <sys/netport.h>
# include <arpa/inet.h>
# include "methods/interface.methodhandler.hpp"
# include "http_handler.interface.hpp"
// # include "methods/get.class.hpp"
// # include "methods/delete.class.hpp"
// # include "methods/post.class.hpp"
// # include "http_handler.interface.hpp"
// # include "http_handler.class.hpp"
// # include "http_parser.class.hpp"
// # include "http/http_parser.class.hpp"
// # include "http/http_response.class.hpp"
# include "structs/client.struct.hpp"
# include "structs/manager.struct.hpp"
# include "structs/request.struct.hpp"
# include "structs/response.struct.hpp"
# include "structs/socket.struct.hpp"
# include "enums.hpp"

#define HashMap std::map

#ifdef MAX_REQUEST_TIMEOUT
# undef MAX_REQUEST_TIMEOUT
#endif

#define MAX_REQUEST_TIMEOUT 5

void    read_header(t_client *client);

#endif