#pragma once

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
# include "http_handler.interface.hpp"
# include "client.struct.hpp"
# include "manager.struct.hpp"
# include "request.struct.hpp"
# include "response.struct.hpp"
# include "socket.struct.hpp"
# include "enums.hpp"
#include <dirent.h>

#define HashMap std::map

#ifdef FD_SETSIZE
# undef FD_SETSIZE
#endif

#define FD_SETSIZE 10000

#ifdef MAX_REQUEST_TIMEOUT
# undef MAX_REQUEST_TIMEOUT
#endif

#define MAX_REQUEST_TIMEOUT 5

void    read_header(t_client *client);
