## How our webserver works
![](https://github.com/hel-mefe/Nginx-similar-webserver-42/blob/main/assets/howto.gif)

This project explores the fundamentals of web server development, focusing on socket programming, HTTP/1.1 protocol, and the request-response cycle. We've crafted a robust and efficient web server from scratch, empowering developers with the knowledge and skills to understand and implement web communication in its entirety.

## Concepts

- **Socket Programming**: Master the art of establishing connections, transmitting data, and managing communication channels between clients and servers.
- **HTTP/1.1 Protocol**: Explore HTTP methods, headers, status codes, and message formats to interpret and construct HTTP messages accurately.
- **Request-response lifecycle**: Understand the heartbeat of web communication, from client requests to server processing and response delivery.
- **Multiplexing with kqueue and epoll**: Utilize efficient event notification mechanisms for handling multiple connections concurrently, enhancing the scalability and performance of the web server.

## Getting Started

To get started with our web server project, follow these steps:

1. **Clone the Repository**: `git clone https://github.com/hel-mefe/Nginx-similar-webserver-42.git`
2. **Build the Project**: Navigate to the project directory and build the project using the Makefile

    ```
	make
    ```
4. **Run the Web Server**: Execute the compiled binary to start the web server.
   ```
	./webserv [configuration_file]
   ```
6. **Access the Web Server**: Open a web browser and navigate to `http://localhost:port` (replace `port` with the port number configured in the server).

## Usage

Our web server supports a range of functionalities, including:

- Serving static content (HTML, CSS, JavaScript, etc.).
- Handling dynamic requests (CGI scripts, server-side scripting, etc.).
- Logging server activity and client requests for analysis and troubleshooting.
- Configurable settings for port number, server root directory, etc.
- GET, POST, PATCH, PUT, TRACE, OPTIONS, DELETE are all supported
- Keep-alive is also supported

## Configuration file documentation

Our configuration file syntax is following the recursive dfs approach, each location's configurations are prioritized over the server block, and each server block has the priority over the http block, exactly as nginx behaves.

### The following tokens are available for the http block:
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

### The following tokens are available for each server block:
 - <b>server_name [string]</b>: specifies the server name
 - <b>listen [number]</b>: specifies the port the virtual server should listen on
 - <b>try_index_files [index1, index2, ... indexN]</b>: takes index files that should be served as indexes
 in case a directory has been requested rather than a normal file
 - <b>try_404_files [file1, file2, ... fileN]</b>: takes 404 files that should be
 served in case the requested path was not found
 - <b>allowed_methods [*] for all the methods or [METHOD1, METHOD2, ... METHOD_N]</b>: takes multiple methods that
 should be supported by the server
 - <b>root [string]</b>: takes the root directory and starts serving files starting from it
 - <b>client_max_connections [number]</b>: takes the number of the maximum simultaneous connections that a server can handle
 - <b>error_page [error_number] [error_file]</b>: takes the number of the error and whenever the server has to respond
 with that http code it servers the provided file as http body response
 - <b>cgi_bin [.extension] [/bin_path]</b>: takes the extension and maps it with the bin cgi path that should be run in case a file
 with the provided extension has been provided
 - <b>max_client_request_timeout</b>: maximum time the server should wait for the client to write something as request,
 the default is 2 seconds in case not provided
 - <b>max_client_body_size</b>: maximum body size the server should consider from the client as http request
 - <b>cgi_max_request_timeout [number]</b>:  specifies number of seconds to wait for the cgi process before it gets killed,
the default value is 30 seconds
  - <b>keep_alive_max_timeout [number]</b>:  specifies the number of seconds to wait in a keep alive connection and the client has written nothing,
the default value is 65 seconds
 - <b>location [location]</b>: defines a location block inside the server block more details about it below

### The following tokens are available for each location block:
 - <b>try_index_files [index1, index2, ... indexN]</b>: takes index files that should be served as indexes
 in case this directory has been requested
 - <b>try_404_files [file1, file2, ... fileN]</b>: takes 404 files that should be served in case the requested path was not found
 - <b>redirect [path]</b>: takes a path and redirects the client to it whenever this location has been requested
 - <b>support_cookies [on/off]</b>: specifies if the requested directory supports cookies or not
 - <b>directory_listing [on/off]</b>: the default value depends on the server block, if specifies on then whenever there is no file
 to serve and this location is requested the directory_listing will run

### Example:

``` json

http
{
	keep_alive_max_timeout 2s
	client_max_uri_size 200
	client_max_body_size 4by
	support_cookies on
	server
	{
		listen 8080
		server_name server_1
		cgi_bin .pl /usr/bin/perl
		root httpdocs/www
		error_page 404 404.png # NOT FOUND
		error_page 400 400.png # BAD REQUEST
		error_page 405 405.png # METHOD NOT ALLOWED
		error_page 414 414.png # URI MAX
		error_page 501 501.png # NOT IMPLEMENTED
		error_page 500 500.png # Internal Server Error
		directory_listing on
		cgi_bin .php bin/php-cgi
	}
}

```

## How to build your own webserver as well?

![](https://media.giphy.com/media/v1.Y2lkPTc5MGI3NjExbXgxczRqZm45NGxnd2x4dXBndWU5N3ZlOWFqMDY1NzdteGd0MXhxdiZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/xT1R9IJlFwp1ImrPig/giphy.gif)

**Understand the HTTP Protocol:** Before diving into web server development, it's crucial to have a solid understanding of the HTTP protocol. Familiarize yourself with HTTP methods (GET, POST, etc.), status codes (200 OK, 404 Not Found, etc.), headers, and the overall request-response cycle. There is no place better than their official RFC's to read from, feel free to read about them from the official RFC's.

**Set Up TCP Socket:** If you're not using a networking library, start by creating a TCP socket to listen for incoming connections. Use the socket() function to create the socket, bind() to associate it with a port, and listen() to start listening for incoming connections.

**Choose a Multiplexing Syscall:** You have the choice between <b>select</b> and <b>poll</b> as a low-level syscalls, but the issue with these ones is that they are linear functions which mean they take O(N) time complexity, you have to iterate over all the sockets in your webserver to figure out which one is ready for I/O operations. There are some high-level ones as <b>Kqueue</b> in FreeBSD and MacOS distributions, in addition to <b>Epoll</b> which is a good multiplexing method if you're on a Linux distribution.

**Accept Incoming Connections:** Once the server socket is set up, use the accept() function to accept incoming connections from clients. This function will return a new socket for each client connection, which you can then use to send and receive data.

**Handle HTTP Requests:** Receive and parse HTTP requests from clients. Extract information such as the request method, requested URL, headers, and body. Depending on the request method and URL, decide how to handle the request (e.g., serve static files, execute dynamic content, etc.).

**Generate HTTP Responses:** Based on the request received, generate an appropriate HTTP response. This may involve reading files from the server's file system, executing server-side scripts, or generating dynamic content. Construct the HTTP response with the correct status code, headers, and content.

**Send Response to Client:** Once the HTTP response is generated, send it back to the client over the established connection. Use the send() or write() function to send the response data over the socket.



