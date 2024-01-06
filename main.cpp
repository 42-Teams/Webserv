/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-maar <ael-maar@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/23 11:34:44 by ael-maar          #+#    #+#             */
/*   Updated: 2024/01/05 20:25:13 by ael-maar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

clientInfo newClientInfo(int clientSocket)
{
    clientInfo client;

    client.clientSocket = clientSocket;
    client.contentLength = -1;
    client.isTransferChunked = false;
    
    return (client);
}

HttpMethod detectMethod(const std::string &request)
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

void updateClientInfo(clientInfo &clientRequest, char buffer[], size_t const bytesRead)
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

bool endsWith(std::string const &requestHeader, std::string const &requestEnd)
{
    return (requestHeader.rfind(requestEnd.c_str(), requestHeader.size(), requestEnd.size()) != std::string::npos);
}

bool isRequestBodyLengthValid(std::string const &requestHeader, size_t contentLength)
{
    size_t locate = requestHeader.find("\r\n\r\n");
    
    std::string body = requestHeader.substr(locate + 4);

    return (body.size() >= contentLength);
}

bool isCompleteMessage(clientInfo &clientRequest)
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

int main(void)
{
    fd_set mainSet, workingSet;
    clientInfoList clientInfos;
    int_v serverSockets;

    FD_ZERO(&mainSet);
    // Create server sockets and add them to servers vector
    for (int i = 80; i <= 85; i++)
    {
        serverSockets.push_back(setupServer(i, BACKLOG));
        FD_SET(serverSockets.back(), &mainSet);
    }

    int max_fds = serverSockets.back(); 
    while (true)
    {
        // copy the main set to the working set, so that when select changes
        // the set of the working, it doesn't affect the original set (mainSet)
        FD_COPY(&mainSet, &workingSet);
        check_error(select(max_fds + 1, &workingSet, NULL, NULL, 0));

        for (int i = 0; i <= max_fds; ++i)
        {
            if (FD_ISSET(i, &workingSet))
            {
                // The server is ready to accept new connection
                if (std::find(serverSockets.begin(), serverSockets.end(), i) != serverSockets.end())
                {
                    int clientSocket = acceptNewConnection(i);
                    check_error(fcntl(clientSocket, F_SETFL, O_NONBLOCK, FD_CLOEXEC));
                    FD_SET(clientSocket, &mainSet);
                    clientInfos.insert(std::make_pair(clientSocket, newClientInfo(clientSocket)));
                    if (clientSocket > max_fds)
                        max_fds = clientSocket;
                }
                else // The client socket is ready for I/O operations
                {
                    char buffer[BUFFER_READ];
                    ssize_t bytesRead = read(i, &buffer, BUFFER_READ);
                    clientInfoIt clientIt = clientInfos.find(i);
                    clientInfo &clientInfo = clientIt->second;

                    if (bytesRead > 0)
                    {
                        // buffer[bytesRead] = '\0';
                        // First time reading from the client, parse the HTTP request header.
                        if (clientInfo.request.empty())
                            updateClientInfo(clientInfo, buffer, bytesRead);
                        else
                            clientInfo.request.append(buffer,bytesRead);
                        if (isCompleteMessage(clientInfo)) // Check if the request header is completed and ready to be handled
                        {
                            handleConnection(clientInfo);
                            clientInfos.erase(clientIt);
                            FD_CLR(i, &mainSet);
                        }
                    } else {
                        clientInfos.erase(clientIt);
                        FD_CLR(i, &mainSet);
                    }
                }
            }
        }
    }

    return (EXIT_SUCCESS);
}
