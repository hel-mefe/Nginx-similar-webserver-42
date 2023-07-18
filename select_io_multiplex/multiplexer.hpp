/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   multiplexer.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fchanane <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/15 21:59:06 by fchanane          #+#    #+#             */
/*   Updated: 2023/07/15 21:59:13 by fchanane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MULTIPLEXER_HPP
# define MULTIPLEXER_HPP

#include<map>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <set>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

typedef struct sockaddr_in address_t;

# define BACKLOG 128
# define BUFFER_SIZE 1024

typedef struct client_s
{
    int serverSocket;
    int state;
    std::string buffer; // buffer is what will change state of client
    //address_t clientAdress;
}   client_t;

#endif

