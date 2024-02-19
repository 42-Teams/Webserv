/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server_utils.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-maar <ael-maar@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/22 19:24:47 by ael-maar          #+#    #+#             */
/*   Updated: 2024/02/06 19:34:50 by ael-maar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

void check_error(int sys, int socketFD)
{
    if (sys == -1)
    {
        close(socketFD);
        throw strerror(errno);
    }
}

int setupServer(int port, int backLog)
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    check_error(serverSocket, serverSocket);

    check_error(fcntl(serverSocket, F_SETFL, O_NONBLOCK, FD_CLOEXEC), serverSocket);
    int opt = 1;
    check_error(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)), serverSocket);
    sockaddr_in serverAddress = {};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);
    check_error(bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)), serverSocket);
    check_error(listen(serverSocket, backLog), serverSocket);

    return (serverSocket);
}

int acceptNewConnection(int serverSocket)
{
    int clientSocket;
    sockaddr_in clientAddress = {};
    socklen_t clientAddressLen = sizeof(clientAddress);

    clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddress), &clientAddressLen);
    check_error(clientSocket, clientSocket);

    return (clientSocket);
}

Server& findServer(std::vector<Server> &conFile, Request &request)
{
    std::string host = request.get_headers()["Host"];
    std::string port = host.substr(host.find(":") + 1);
    host = host.substr(0, host.find(":"));
    for (size_t i = 0; i < conFile.size(); i++)
    {
        if (conFile[i].get_port() == std::atoi(port.c_str()) && conFile[i].get_name() == host)
            return conFile[i];
    }
    for (size_t i = 0; i < conFile.size(); i++)
    {
        if (conFile[i].get_port() == std::atoi(port.c_str()))
            return conFile[i];
    }
    return conFile[0];
}

void handleConnection(std::vector<Server> &conFile, clientInfo &clientInfo)
{
    Request request(clientInfo.request);
    std::string connection_status = request.get_headers()["Connection"];
    clientInfo.keepAlive = false;
    if (connection_status == "keep-alive")
        clientInfo.keepAlive = true;
    Response response(request,findServer(conFile, request));
    clientInfo.response = response.get_response();
}
