// # include "socket.hpp"

// // mainly used for methods
// std::string get_upper_case(std::string s)
// {
//     std::string res = s;

//     for (int i = 0; i < sz(s); i++)
//     {
//         if (isalpha(s[i]) && islower(s[i]))
//             res[i] = toupper(s[i]);
//     }
//     return res ;
// }

// // mainly used for request parameters
// std::string get_lower_case(std::string s)
// {
//     std::string res = s;

//     for (int i = 0; i < sz(s); i++)
//     {
//         if (isalpha(s[i]) && isupper(s[i]))
//             res[i] = tolower(s[i]);
//     }
//     return res;
// }

// void    parse_first_line(t_request *req)
// {
//     std::string line = req->lines[0];
//     std::string method, path, http_version;
//     int i = 0, k = 0;

//     for (; i < sz(line) && isspace(line[i]); i++);
//     k = i;
//     for (; i < sz(line) && !isspace(line[i]); i++);
//     method = line.substr(k, i);
//     for (; i < sz(line) && isspace(line[i]); i++);
//     int a = i;
//     for (; i < sz(line) && !isspace(line[i]); i++);
//     path = line.substr(a, i - a);
//     for (; i < sz(line) && isspace(line[i]); i++);
//     a = i;
//     for(; i < sz(line) && !isspace(line[i]); i++);
//     http_version = line.substr(a, i - a);
//     method = get_upper_case(method);
//     http_version = get_upper_case(http_version);
//     req->http_version = http_version;
//     req->method = method;
//     req->path = path;
// }
// void    parse_header(t_request *req)
// {
//     parse_first_line(req);
//     for (int i = 1; i < sz(req->lines); i++)
//     {
//         std::string line = req->lines[i], first, second;
    
//         int j = 0, a = 0;
//         for (; j < sz(line) && isspace(line[j]); j++);
//         a = j;
//         for (; j < sz(line) && !isspace(line[j]) && line[j] != ':'; j++);
//         first = get_lower_case(line.substr(a, j - a));
    
//         for (; j < sz(line) && isspace(line[j]); j++);
//         j += (j < sz(line) && line[j] == ':');
//         for (; j < sz(line) && isspace(line[j]); j++);

//         a = j;
//         for (; j < sz(line) && !isspace(line[j]); j++)
//         second = get_lower_case(line.substr(a, j - a + 1));
//         req->request_map.insert(std::make_pair(first, second));
//     }
//     std::cout << "***** PRINTING REQUEST MAP *****" << std::endl;
//     for (auto x: req->request_map)
//         std::cout << x.first << " " << x.second << std::endl;
//     std::cout << "***** END PRINTING REQUEST MAP *****" << std::endl;
// }

// void    build_client_response(t_client *client)
// {
//     t_request   *req;
//     t_response  *res;

//     req = client->request;
//     if (req->method == "GET")
//         client->state = SERVING_GET;
//     else if (req->method == "POST")
//         client->state = SERVING_POST;
//     else if (req->method == "DELETE")
//         client->state = SERVING_DELETE;
//     res = client->response;
//     res->http_version = "HTTP/1.1";
//     res->status_code = "301";
//     res->status_line = "Moved Permanently";
//     // res->add("connection", "keep-alive");
//     // res->add("transfer-encoding", "chunked");
//     // res->add("content-type", "text/html");    
//     res->add("location", "gohan");
//     res->write_response_in_socketfd(client->fd);
//     client->state = SERVED;
//     res->filename = "index.html";
//     std::cout << "MY SERVER -> " << client->server->server_configs->port << std::endl;
// }

// void    read_header(t_client *client)
// {
//     char buff[MAX_BUFFER_SIZE];
//     int bytes;
//     std::string lines;
//     bool header_complete = false ;

//     bzero(buff, MAX_BUFFER_SIZE);
//     bytes = read(client->fd, buff, MAX_BUFFER_SIZE);
//     if (!bytes)
//     {
//         client->state = SERVED;
//         return ;
//     }
//     lines = buff;
//     std::stringstream strm(lines);
//     while (strm.good() && !header_complete)
//     {
//         std::string line;
//         std::getline(strm, line);
//         if (line[sz(line) - 1] == '\r')
//             line = line.substr(0, sz(line) - 1);
//         header_complete = !sz(line);
//         if (sz(line) && !header_complete)
//             client->request->lines.push_back(line);
//     }
//     if (header_complete)
//     {
//         parse_header(client->request);
//         build_client_response(client);
//     }
// }