## How this webserver works
![](https://github.com/hel-mefe/Nginx-similar-webserver-42/blob/main/assets/howto.gif)

This project explores the fundamentals of web server development, focusing on socket programming, HTTP/1.1 protocol, and the request-response cycle. We've crafted a robust and efficient web server from scratch, empowering developers with the knowledge and skills to understand and implement web communication in its entirety.

## Concepts

- **Socket Programming**: Master the art of establishing connections, transmitting data, and managing communication channels between clients and servers.
- **HTTP/1.1 Protocol**: Explore HTTP methods, headers, status codes, and message formats to interpret and construct HTTP messages accurately.
- **Request-Response Cycle**: Understand the heartbeat of web communication, from client requests to server processing and response delivery.

## Getting Started

To get started with our web server project, follow these steps:

1. **Clone the Repository**: `git clone https://github.com/yourusername/web-server.git`
2. **Build the Project**: Navigate to the project directory and build the project using your preferred build system (e.g., Makefile).
3. **Run the Web Server**: Execute the compiled binary to start the web server.
4. **Access the Web Server**: Open a web browser and navigate to `http://localhost:port` (replace `port` with the port number configured in the server).

## Usage

Our web server supports a range of functionalities, including:

- Serving static content (HTML, CSS, JavaScript, etc.).
- Handling dynamic requests (CGI scripts, server-side scripting, etc.).
- Logging server activity and client requests for analysis and troubleshooting.
- Configurable settings for port number, server root directory, etc.

## CLI Guide

The following tokens are available for the http block:
 - <b>root [string]:</b> defines the root directory
 - <b>allowed_methods [METHOD1, METHOD2 ...]</b>:  or * asterisk for all the methods, identifies the
 http methods allowed in the whole webserver
 - <b>client_max_body_size [number]</b>: identifies the max body size per request
 - <b>client_max_request_timeout [number]</b>: identifies the max request time the webserver waits
 for the request
  - <b>client_max_uri_size [number]</b>: identifies the max uri size in term of how many characters
 the webserver can accept
  - <b>cgi_max_request_timeout [number]</b>:  specifies number of seconds to wait for the cgi process before it gets killed,
the default value is 30 seconds
  - <b>keep_alive_max_timeout [number]</b>:  specifies the number of seconds to wait in a keep alive connection when the client has written nothing,
the default value is 65 seconds
 - <b>multiplexer [takes one of these 'kqueue' or 'epoll' or 'poll' or 'select']</b>: specifies the multiplexer used for simultaneous
 connections, kqueue is the default one for FreeBSD and Apple distributions meanwhile Epoll is the default
 one for Linux distributions
 - <b>support_cookies [on/off]</b>:  specifies if the webserver supports cookies or not
 - <b>proxy_cache [on/off]</b>:  specifies if the server should serve requests from the cache or not, the default is off
 - <b>proxy_cache_register [on/off]</b>:  specifies if the server should register the requests that require some processing for future use  the default is off
 - <b>proxy_cache_max_time [time] ex. (10s, 10m, 10h, 10d)</b>:  specifies the period of time a request should get cached, 3 days is the default
 - <b>proxy_cache_max_size [size] ex. (10by, 10kb, 10mb, 10gb)</b>:  specifies max size of the caches that the server should never surpass,  12mb is the default

The following tokens are available for each server block:
 - server_name [string]: specifies the server name
 - listen [number]: specifies the port the virtual server should listen on
 - try_index_files [index1, index2, ... indexN]: takes index files that should be served as indexes
 in case a directory has been requested rather than a normal file
 - try_404_files [file1, file2, ... fileN]: takes 404 files that should be
 served in case the requested path was not found
 - allowed_methods [*] for all the methods or [METHOD1, METHOD2, ... METHOD_N]: takes multiple methods that
 should be supported by the server
 - root [string]: takes the root directory and starts serving files starting from it
 - client_max_connections [number]: takes the number of the maximum simultaneous connections that a server can handle
 - error_page [error_number] [error_file]: takes the number of the error and whenever the server has to respond
 with that http code it servers the provided file as http body response
 - cgi_bin [.extension] [/bin_path]: takes the extension and maps it with the bin cgi path that should be run in case a file
 with the provided extension has been provided
 - max_client_request_timeout: maximum time the server should wait for the client to write something as request,
 the default is 2 seconds in case not provided
 - max_client_body_size: maximum body size the server should consider from the client as http request
  - cgi_max_request_timeout [number]:  specifies number of seconds to wait for the cgi process before it gets killed,
the default value is 30 seconds
  - keep_alive_max_timeout [number]:  specifies the number of seconds to wait in a keep alive connection and the client has written nothing,
the default value is 65 seconds
 - location [location]: defines a location block inside the server block more details about it below

The following tokens are available for each location block:
 - try_index_files [index1, index2, ... indexN]: takes index files that should be served as indexes
 in case this directory has been requested
 - try_404_files [file1, file2, ... fileN]: takes 404 files that should be served in case the requested path was not found
 - redirect [path]: takes a path and redirects the client to it whenever this location has been requested
 - support_cookies [on/off]: specifies if the requested directory supports cookies or not
 - directory_listing [on/off]: the default value depends on the server block, if specifies on then whenever there is no file
 to serve and this location is requested the directory_listing will run
