/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-maar <ael-maar@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/08 12:24:42 by ael-maar          #+#    #+#             */
/*   Updated: 2024/02/06 19:36:04 by ael-maar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

ServerManager::ServerManager(std::vector<Server> &conFile): conFile(conFile)
{
    FD_ZERO(&mainReadSet);
    FD_ZERO(&mainWriteSet);
    max_fds = 0;
}

void ServerManager::setupServers()
{
    for (size_t i = 0; i < conFile.size(); ++i)
    {
        try
        {
            int socketFD = setupServer(conFile.at(i).get_port(), BACKLOG); // create new socket for the server
            FD_SET(socketFD, &mainReadSet);
            serverSockets.push_back(socketFD);
        }
        catch(const char *error)
        {
            std::cerr << "ERROR: " << error << '\n';
        }
    }
    max_fds = serverSockets.back();
}


clientInfo ServerManager::newClientInfo(int clientSocket) const
{
    clientInfo client;

    client.clientSocket = clientSocket;
    client.contentLength = -1;
    client.isTransferChunked = false;
    client.responseBytesSent = 0;
    client.keepAlive = false;
    client.lastActivity = time(NULL);
    return (client);
}

void ServerManager::updateClientInfo(clientInfo &clientRequest, char buffer[], size_t const bytesRead)
{
    size_t locate;
    std::string &requestHeader = clientRequest.request;

    requestHeader.append(buffer, bytesRead);
    // Locate the method in the request
    clientRequest.method = detectMethod(requestHeader);
    // Locate if the request is chunked for POST request
    locate = requestHeader.find("Transfer-Encoding: chunked");
    if (locate != std::string::npos)
        clientRequest.isTransferChunked = true;
    // Locate and extract the content length
    locate = requestHeader.find("Content-Length:");
    if (locate != std::string::npos)
        clientRequest.contentLength = std::atoi(requestHeader.substr(locate + strlen("Content-Length:")).c_str());
}

HttpMethod ServerManager::detectMethod(const std::string &request) const
{
    std::string method = request.substr(0, request.find(" "));
    if (method == "GET")
        return (GET);
    if (method == "POST")
        return (POST);
    if (method == "DELETE")
        return (DELETE);
    return (NONE);
}


void ServerManager::handleNewConnection(int socket)
{
    int clientSocket = acceptNewConnection(socket);
    check_error(fcntl(clientSocket, F_SETFL, O_NONBLOCK, FD_CLOEXEC), clientSocket);
    FD_SET(clientSocket, &mainReadSet);
    clientInfos.insert(std::make_pair(clientSocket, newClientInfo(clientSocket)));
    if (clientSocket > max_fds)
        max_fds = clientSocket;
}

bool ServerManager::endsWith(std::string const &requestHeader, std::string const &requestEnd) const
{
    return (requestHeader.rfind(requestEnd.c_str(), requestHeader.size(), requestEnd.size()) != std::string::npos);
}

bool ServerManager::isRequestBodyLengthValid(std::string const &requestHeader, size_t contentLength) const
{
    size_t locate = requestHeader.find("\r\n\r\n");

    size_t bodyLen = requestHeader.size() - (locate + 4);

    return (bodyLen >= contentLength);
}

bool ServerManager::isCompleteMessage(clientInfo &clientRequest) const
{
    switch(clientRequest.method)
    {
        case POST:
            if (clientRequest.isTransferChunked)
                return (endsWith(clientRequest.request, "\r\n\r\n0\r\n\r\n"));
            else if (clientRequest.contentLength != -1)
                return (isRequestBodyLengthValid(clientRequest.request, clientRequest.contentLength));
            else
                return (endsWith(clientRequest.request, "\r\n\r\n"));
        default:
            return (endsWith(clientRequest.request, "\r\n\r\n")); // For all the other methods
    }
}

void ServerManager::handleIncomingData(int socket)
{
    char buffer[READ_BUFFER];
    ssize_t bytesRead = read(socket, &buffer, READ_BUFFER);
    clientInfoIt clientIt = clientInfos.find(socket);
    clientInfo &clientInfo = clientIt->second;

    if (bytesRead > 0)
    {
        // Update the last activity time for the client
        clientInfo.lastActivity = time(NULL);
        // First time reading from the client, parse the HTTP request header.
        if (clientInfo.request.empty())
            updateClientInfo(clientInfo, buffer, bytesRead);
        else
            clientInfo.request.append(buffer,bytesRead);
        if (isCompleteMessage(clientInfo)) // Check if the request header is completed and ready to be handled
        {
            handleConnection(this->conFile, clientInfo);
            clientInfo.request.clear();
            FD_SET(socket, &mainWriteSet);
            FD_CLR(socket, &mainReadSet);
        }
    } else {
        clientInfos.erase(clientIt);
        FD_CLR(socket, &mainReadSet);
        close(socket);
    }
}

void ServerManager::handleSendingData(int socket)
{
    clientInfoIt clientIt = clientInfos.find(socket);
    clientInfo &clientInfo = clientIt->second;
    size_t &bytesSent = clientInfo.responseBytesSent;

    ssize_t bytesWriting = write(socket, clientInfo.response.c_str() + bytesSent, clientInfo.response.size() - bytesSent);

    if (bytesWriting == -1)
    {
        FD_CLR(socket, &mainWriteSet);
        clientInfos.erase(clientIt);
        close(socket);
        return ;
    }
    bytesSent += bytesWriting;
    clientInfo.lastActivity = time(NULL); // Update the last activity time for the client
    if (bytesSent >= clientInfo.response.size())
    {
        FD_CLR(socket, &mainWriteSet);
        if (clientInfo.keepAlive){
            FD_SET(socket, &mainReadSet);
            clientInfo.response.clear();
            bytesSent = 0;
        }else{
            clientInfos.erase(clientIt);
            close(socket);
        }
    }
}

bool ServerManager::socketReadyForRead(int socket) const
{
    return (FD_ISSET(socket, &workingReadSet));
}

bool ServerManager::socketReadyForWrite(int socket) const
{
    return (FD_ISSET(socket, &workingWriteSet));
}

bool ServerManager::isServerSocket(int socket) const
{
    return (std::find(serverSockets.begin(), serverSockets.end(), socket) != serverSockets.end());
}

void ServerManager::filterInactiveConnections() {
    time_t now = time(NULL);
    for (clientInfoIt it = clientInfos.begin(); it != clientInfos.end();) {
        if (now - it->second.lastActivity > INACTIVITY_TIMEOUT) {
            int socket = it->first;
            FD_CLR(socket, &mainReadSet);
            FD_CLR(socket, &mainWriteSet);
            close(socket);
            clientInfos.erase(it++);
        } else {
            ++it;
        }
    }
}

void ServerManager::run()
{
    signal(SIGPIPE, SIG_IGN);
    while (true)
    {
        filterInactiveConnections(); // Filter inactive connections
        FD_COPY(&mainReadSet, &workingReadSet);
        FD_COPY(&mainWriteSet, &workingWriteSet);
        // Set up the timeval struct for the select() call.
        struct timeval timeout;
        timeout.tv_sec = INACTIVITY_TIMEOUT;
        timeout.tv_usec = 0;
        try
        {
            check_error(select(max_fds + 1, &workingReadSet, &workingWriteSet, NULL, &timeout), -1); // check ready socket file descriptors.

            for (int socketFD = 3; socketFD <= max_fds; ++socketFD)
            {
                try
                {
                    if (socketReadyForRead(socketFD))
                    {
                        if (isServerSocket(socketFD))
                            handleNewConnection(socketFD);
                        else
                            handleIncomingData(socketFD);
                    }
                    else if (socketReadyForWrite(socketFD))
                        handleSendingData(socketFD);
                }
                catch(const char *error)
                {
                    std::cerr << error << '\n';
                }
            }
        }
        catch(const char *error)
        {
            std::cerr << error << '\n';
        }
    }

}
