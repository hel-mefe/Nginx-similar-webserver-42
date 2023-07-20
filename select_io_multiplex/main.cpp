/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fchanane <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/15 21:55:00 by fchanane          #+#    #+#             */
/*   Updated: 2023/07/15 21:55:02 by fchanane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "multiplexer.hpp"

std::map<int, std::string> staticSockets(void)
{
    std::map<int, std::string> sockets;
    sockets.insert(std::make_pair(8080, "127.0.0.1"));
    sockets.insert(std::make_pair(7070, "127.0.0.1"));
    sockets.insert(std::make_pair(9090, "127.0.0.1"));
    return (sockets);
}

int createSocket(size_t size, std::vector<int>& socketFds)
{
    for (size_t i = 0; i < size; i++)
    {
        int serverFd = socket(AF_INET, SOCK_STREAM, 0); // create a socket
        if (serverFd < 0)
        {
            std::cerr<<"Failed to create the socket"<<std::endl;
            return 1;
        }
        int opt = 1;
        if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)))
        {
            std::cerr<<"Error occured while setting options"<<std::endl;
            return 1;
        }
        int mode = fcntl(serverFd, F_GETFL, 0);
        if (mode == -1)
        {
            std::cerr<<"Error occured while getting socket mode"<<std::endl;
            return 1;
        }
        if (fcntl(serverFd, F_SETFL, mode | O_NONBLOCK) == -1)
        {
            std::cerr<<"Error occured while setting socket to non_blocking"<<std::endl;
            return 1;
        }
        socketFds.push_back(serverFd);
    }
    return 0;
}

void    printListener(address_t serverAdress)
{
    std::cout<< "*** Listening on ADDRESS: " 
    << inet_ntoa(serverAdress.sin_addr) 
    << " PORT: " << ntohs(serverAdress.sin_port) 
    << " ***\n\n";
}

int bindSocket(std::vector<address_t>& add, std::vector<int> socketFds, std::map<int, std::string> sockets)
{
    size_t i = 0;
    for (std::map<int, std::string>::iterator it = sockets.begin(); it != sockets.end(); it++)
    {
        address_t serverAdress;
        //memset(&serverAdress, 0, sizeof(serverAdress));
        serverAdress.sin_family = AF_INET;
        serverAdress.sin_port = htons(it->first);
        serverAdress.sin_addr.s_addr = inet_addr(it->second.c_str());
        add.push_back(serverAdress);
                int bindResult = bind(socketFds[i], (struct sockaddr*)&serverAdress, sizeof(serverAdress));
        if (bindResult < 0)
        {
            std::cerr << "Error binding socket\n";
            return 1;
        }
        int listenResult = listen(socketFds[i], BACKLOG);
        if (listenResult < 0)
        {
            std::cerr<< "Error listening on socket\n";
            return 1;
        }
        //printListener(serverAdress);
        i++;
    }
    return (0);
}

void set_fds(fd_set *readfds, fd_set *writefds, std::vector<int> socketFds, std::map<int, client_t>& clients)
{
    FD_ZERO(readfds);
    FD_ZERO(writefds);
    for (size_t i = 0; i < socketFds.size(); i++)
    {
        FD_SET(socketFds[i], readfds);
    }
    std::map<int, client_t>::iterator it;
    for (it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->second.state == 1)
            FD_SET(it->first, readfds);
        else if (it->second.state == 2)
            FD_SET(it->first, writefds);
    }
    // for (size_t i = 0; i < clientFds.size(); i++)
    // {
    //     // set read and write fds by phase
    //     if (STATE == 1)
    //     {
    //         FD_SET(clientFds[i], readfds);
    //     }
    //     else if (STATE == 2)
    //     {
    //         FD_SET(clientFds[i], writefds);
    //     }
    // }
}

int ft_recv(std::map<int, client_t>& clients, int clientFd)
{
    //client_t client = clients[clientFd];
    char buffer[BUFFER_SIZE];
    //std::cout<<"Client "<<clientFd<<" is ready to read"<<std::endl;
    int readResult = recv(clientFd, buffer, BUFFER_SIZE, 0);
    if (readResult < 0)
    {
        std::cerr<<"Error reading from socket\n";
        return 1;
    }
    else if (readResult == 0)
    {
        //std::cout<<"Client disconnected\n";
        return 2;
    }
    std::cout<<"Received message: "<<buffer<<std::endl;//<<buffer<<std::endl;
    clients[clientFd].buffer = buffer;
    clients[clientFd].state = 2; // test to transform to write
    return 0;
}

int ft_send(std::map<int, client_t>& clients, int clientFd)
{
    //std::cout<<"HEERE"<<std::endl;
    // std::string message = clients[clientFd].buffer;
    // int sendResult = send(clientFd, message.c_str(), message.length(), 0);
    // if (sendResult < 0)
    // {
    //     //std::cerr<<"Error sending message\n";
    //     return 1;
    // }
    // //std::cout<<"Message sent: "<<message<<std::endl;
    // clients[clientFd].state = 3; // test to remove client
    // return 0;
    std::string response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: 12\r\n\r\n"
                           "Hello, World!";
    int sendResult = send(clientFd, response.c_str(), response.length(), 0);
    if (sendResult <= 0)
    {
        std::cerr<<"Error sending message\n";
        return 1;
    }
    //std::cout<<"Message sent: "<<response<<std::endl;
    clients[clientFd].state = 3; // test to remove client
    //std::cout<<"Client state is "<<clients[clientFd].state<<std::endl;
    return 0;
}

void addClient(int fd, int serverSocket, std::map<int, client_t>& clients)
{
    client_t client;
    client.serverSocket = serverSocket;
    client.state = 1;
    client.buffer = "";
    clients.insert(std::make_pair(fd, client));
}

void removeClient(int clientFd, std::map<int, client_t>& clients)
{
    // if (!IN_MAP(clients_map, fd))
    //     return false ;
    // if (close(fd))
    //     std::cout << RED_BOLD << "[ FD WAS NOT CLOSED ]" << std::endl;
    // t_client *data = clients_map[fd];
    // std::cout << "REMOVE CLIENT" << std::endl;
    // printClientsMap();
    // int i = data->slot;
    // fds[i].fd = -1;
    // fds[i].events = 0;
    // fds[i].revents = 0;
    // // move_clients_left(i);
    // int b = clients_map.erase(fd);
    // std::cout << b << " has been erased! => " << fd << std::endl;
    // std::cout << "AFTER REMOVE" << std::endl;
    // printClientsMap();
    // add_slot(i);
    // delete data;
    // return true ;
    std::cout<<"Client "<<clientFd<<" disconnected from server "<<std::endl;
    //std::cout<<"Client removed\n"<<std::endl;
    close(clientFd);
    clients.erase(clientFd);
}

int main(void)
{
    //char buffer[BUFFER_SIZE];

    std::map<int, client_t> clients; // this to store client need to be map<int, client_t>

    //std::vector<int> clientFds;
    std::map<int, std::string> sockets = staticSockets(); // has the pair <port, IP>, this only for test
    std::vector<int> socketFds; // this to store fds that will be assigned to sockets
    createSocket(sockets.size(), socketFds); // created the sockets, need check for error "1"
    std::vector<address_t> add;
    bindSocket(add, socketFds, sockets);
    // for (size_t i = 0; i < add.size(); i++)
    // {
    //     std::cout<<"\033[32mLISTENER WITH SOCKET "<<socketFds[i]<<" :"<<std::endl;
    //     printListener(add[i]);
    // }
    fd_set readfds;
    fd_set writefds;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);

    int maxFd = 0;
    set_fds(&readfds, &writefds, socketFds, clients); //HTTP_STATE
    maxFd = *std::max_element(socketFds.begin(), socketFds.end());
    while (true)
    {
        set_fds(&readfds, &writefds, socketFds, clients);
        int activity = select(maxFd + 1, &readfds, &writefds, NULL, NULL);
        if (activity < 0)
        {
            std::cerr<<"SELECT ERROR"<<std::endl;
            return 1;
        }
        for (int i = 0; i <= maxFd; i++)
        {
            //std::cout <<"maxfd is : " << maxFd  << "     i is " << i << std::endl;
            if (FD_ISSET(i, &readfds) || FD_ISSET(i, &writefds))
            {
                //std::cout << "Socket " << i << " is ready" << std::endl;
                std::vector<int>::iterator its = std::find(socketFds.begin(), socketFds.end(), i);
                if (its != socketFds.end()) // its a serverSocket
                {
                    //std::cout << i << " is a server socket" << std::endl;
                    // accept new client on this socket
                    int clientSocket = accept(i, NULL, NULL);
                    if (clientSocket < 0)
                    {
                        std::cerr<<"Failed to create a new client connection"<<std::endl;
                        continue ;
                    }
                    std::cout<<"Client "<<clientSocket<<" connected on server "<<i<<std::endl;
                    //std::cout<<"1 - New client connected on socket "<<clientSocket<<" with server with socket "<<i<<std::endl;
                    FD_SET(clientSocket, &readfds);
                    addClient(clientSocket, i, clients); // not adapted yet to the full code
                    //std::cout<<"2 - New client connected on socket "<<clientSocket<<" with server with socket "<<clients[clientSocket].serverSocket<<std::endl;
                    //clientFds.push_back(clientSocket);
                    if (clientSocket > maxFd)
                    {
                        maxFd = std::max(maxFd, clientSocket);
                        break ;
                    }
                }
                std::map<int, client_t>::iterator itc = clients.find(i);
                if (itc != clients.end()) // is a client
                {
                    //std::cout<<"Client state "<<clients[i].state<<std::endl;
                    //std::cout << i << " is a client socket" << std::endl;
                    //handle_client(t_manager *manager, SOCKET fd)
                    if (FD_ISSET(i, &readfds))
                    {
                        //ft_recv(clients, i);
                        char buffer[BUFFER_SIZE];
                        recv(i, buffer, BUFFER_SIZE, 0); // errors handling
                        std::cout << "Received message: \n" << buffer << std::endl;
                        clients[i].state = 2;
                    }
                    else if (FD_ISSET(i, &writefds))
                    {
                        ft_send(clients, i);
                    }

                }
                if (clients[i].state == 3)
                    removeClient(i, clients);
            }
        }
    }
    return (0);
}
