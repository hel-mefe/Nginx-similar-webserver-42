#include "../includes/trace.class.hpp"
#include "../includes/http_handler.utils.hpp"
#include <sys/types.h>
#include <dirent.h>

Trace::Trace(void) {std::cout << "TRACE CREATED" << std::endl;}

Trace::~Trace(void) {}

std::string Trace::getServerName(t_client *client)
{
    std::string serverName;
    serverName = client->server->server_configs->server_name;
    return (serverName);
}

std::string Trace::getServerVersion(t_client *client)
{
    std::string httpVersion = client->request->http_version;
    std::string serverVersion = httpVersion.substr(5, 3);
    return (serverVersion);
}

std::vector<std::string> Trace::getForbiddenHeades(void)
{
    std::vector<std::string> forbiddenHeaders; // for security reasons these headers should not be sent back to the client
    forbiddenHeaders.push_back("authorization");
    forbiddenHeaders.push_back("cookie");
    forbiddenHeaders.push_back("set-cookie");
    forbiddenHeaders.push_back("www-authenticate");
    forbiddenHeaders.push_back("proxy-authenticate");
    forbiddenHeaders.push_back("proxy-authorization");
    forbiddenHeaders.push_back("accept-encoding");
    forbiddenHeaders.push_back("location");
    forbiddenHeaders.push_back("content-encoding");
    forbiddenHeaders.push_back("transfer-encoding");
    forbiddenHeaders.push_back("retry-after");
    
    return (forbiddenHeaders);
}

std::string Trace::setBody(t_client *client, std::vector<std::string> forbiddenHeaders)
{
    //(void)forbiddenHeaders;
    std::map<std::string, std::string>::iterator it;
    std::string body;
    for(it = client->request->request_map.begin(); it != client->request->request_map.end(); it++)
    {
        //std::cout<<it->first<<" = "<<it->second<<std::endl;
        std::vector<std::string>::iterator itf = std::find(forbiddenHeaders.begin(), forbiddenHeaders.end(), it->first);
        if (itf == forbiddenHeaders.end())
        {
            std::string addedToBody = it->first + ": " + it->second + "\n";
            body += addedToBody;
        }
    }
    return (body);
}

std::string Trace::getBodySize(std::string body)
{
    std::string bodySize = std::to_string(body.size());
    return (bodySize);
}

char* Trace::setDateHeader(char* dateStr, int size)
{
    std::time_t now = std::time(nullptr);
    std::strftime(dateStr, size, "Date: %a, %d %b %Y %H:%M:%S GMT\r\n", std::gmtime(&now));
    return (dateStr);
}

void Trace::serve_client(t_client *client)
{
    int clientFd = client->fd;
    char dateStr[128];
    std::cout<<setDateHeader(dateStr, 128)<<std::endl;
    std::string body = setBody(client, getForbiddenHeades());
    std::string bodySize = getBodySize(body);
    std::string via = getServerVersion(client) + " " + getServerName(client);
    std::string response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: " + bodySize +"\r\n"
                           "Via: " + via + "\r\n"
                           "" + std::string(dateStr) + "\r\n";
    response += body;
    int sendResult = send(clientFd, response.c_str(), response.length(), 0);
    if (sendResult <= 0)
    {
        std::cerr<<"Error sending message\n";
        return;
    }
    client->state = SERVED;
}