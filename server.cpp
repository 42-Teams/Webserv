/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-maar <ael-maar@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/22 19:24:47 by ael-maar          #+#    #+#             */
/*   Updated: 2024/01/03 19:09:04 by ael-maar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
// create a server (socket) that listens to multipe client connections

void check_error(int sys)
{
    if (sys == -1)
    {
        std::cerr << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

int setupServer(int port, int backLog)
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    check_error(serverSocket);

    int opt = 1;
    check_error(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)));
    check_error(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)));
    sockaddr_in serverAddress = {};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);
    check_error(bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)));
    check_error(listen(serverSocket, backLog));

    return (serverSocket);
}

int acceptNewConnection(int serverSocket)
{
    int clientSocket;
    sockaddr_in clientAddress = {};
    socklen_t clientAddressLen = sizeof(clientAddress);

    clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddress), &clientAddressLen);
    check_error(clientSocket);

    return (clientSocket);
}

void handleConnection(clientInfo &clientInfo)
{
    std::cout << clientInfo.request << std::endl;

    std::string message = "Hello world";

    send(clientInfo.clientSocket, message.c_str(), message.size(), 0);
    close(clientInfo.clientSocket);
}
