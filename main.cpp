/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-maar <ael-maar@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/23 11:34:44 by ael-maar          #+#    #+#             */
/*   Updated: 2024/01/04 17:44:35 by ael-maar         ###   ########.fr       */
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
    size_t locateMethod = request.find("POST");
    if (locateMethod != std::string::npos)
        return (POST);
    locateMethod = request.find("GET");
    if (locateMethod != std::string::npos)
        return (GET);
    locateMethod = request.find("DELETE");
    if (locateMethod != std::string::npos)
        return (DELETE);
    return (NONE);
}

bool isTransferEncodingChunked(const std::string &request)
{
    size_t locate = request.find("Transfer-Encoding:");
    if (locate != std::string::npos)
    {
        std::string value = request.substr(locate + strlen("Transfer-Encoding:"));
        // Find the end of the line
        size_t endOfLine = value.find("\r\n");
        if (endOfLine != std::string::npos)
        {
            value = value.substr(0, endOfLine);
            // Remove leading and trailing whitespaces
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            if (value == "chunked")
                 return (true);
        }
    }
    return (false);
}

int extractContentLength(const std::string &request)
{
    size_t locate = request.find("Content-Length:");
    if (locate != std::string::npos)
        return (atoi(request.substr(locate + strlen("Content-Length:")).c_str()));
    return (-1);
}

void updateClientInfo(clientInfo &clientRequest, char buffer[])
{
    size_t locate;
    std::string &requestHeader = clientRequest.request;

    requestHeader += buffer;
    // Locate the method in the request
    clientRequest.method = detectMethod(requestHeader);
    // Locate if the request is chunked for POST request
    clientRequest.isTransferChunked = isTransferEncodingChunked(requestHeader);
    // Locate and extract the content length
    clientRequest.contentLength = extractContentLength(requestHeader);
}

bool endsWith(std::string const &requestHeader, std::string const &requestEnd)
{
    return (requestHeader.find(requestEnd) != std::string::npos);
}

bool isRequestBodyLengthValid(std::string const &requestHeader, int const contentLength)
{
    size_t locate = requestHeader.find("\r\n\r\n");

    size_t requestBodyLen = strlen(requestHeader.c_str() + locate + strlen("\r\n\r\n"));

    return (requestBodyLen >= contentLength);
}

bool isCompleteMessage(clientInfo &clientRequest)
{
    switch(clientRequest.method)
    {
        case GET:
            return (endsWith(clientRequest.request, "\r\n\r\n"));
        case POST:
            if (clientRequest.isTransferChunked)
                return (endsWith(clientRequest.request, "\r\n0\r\n"));
            else if (clientRequest.contentLength != -1)
                return (isRequestBodyLengthValid(clientRequest.request, clientRequest.contentLength));
            else
                return (endsWith(clientRequest.request, "\r\n\r\n"));
        default:
            return (true); // for DELETE method (not yet handled)
    }
}

int main(void)
{
    fd_set mainSet, workingSet;
    clientInfoList clientInfos;
    int_v serverSockets;

    // Create server sockets and add them to servers vector
    for (int i = 0; i < 5; i++)
        serverSockets.push_back(setupServer(80, BACKLOG));

    FD_ZERO(&mainSet);
    // Add the server sockets to the set (mainSet)
    for (int i = 0; i < serverSockets.size(); ++i)
    {
        check_error(fcntl(serverSockets[i], F_SETFL, O_NONBLOCK, FD_CLOEXEC));
        FD_SET(serverSockets[i], &mainSet);
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
                        buffer[bytesRead] = '\0';
                        // First time reading from the client, parse the HTTP request header.
                        if (clientInfo.request.empty())
                            updateClientInfo(clientInfo, buffer);
                        else
                            clientInfo.request += buffer;
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
