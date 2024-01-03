/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-maar <ael-maar@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/23 11:33:05 by ael-maar          #+#    #+#             */
/*   Updated: 2024/01/03 19:12:37 by ael-maar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <fcntl.h>
#include <unordered_map>
#include <utility>

# define BUFFER_READ 1024

/************************/
/*     SERVER PART      */
/************************/

# define BACKLOG 10
typedef std::vector<int> int_v;

typedef enum HttpMethod {
    GET,
    POST,
    DELETE,
    NONE
} HttpMethod;

// typedef enum PosType
// {
//     CHUNKED,
//     CONTENT_LENGTH,
//     NONE
// } PosType;

typedef struct clientInfo
{
    int clientSocket;
    HttpMethod method;
    bool isTransferChunked;
    int contentLength;
    std::string request;
    std::string response;
} clientInfo;

typedef std::unordered_map<int, clientInfo> clientInfoList;
typedef clientInfoList::iterator clientInfoIt;

int  setupServer(int port, int backLog);
int  acceptNewConnection(int serverSocket);
void handleConnection(clientInfo &clientInfo);
void check_error(int sys);

#endif