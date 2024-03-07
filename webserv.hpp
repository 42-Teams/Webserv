/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-maar <ael-maar@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/23 11:33:05 by ael-maar          #+#    #+#             */
/*   Updated: 2024/02/06 19:32:53 by ael-maar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file webserv.hpp
 * @brief Header file for the web server implementation.
 *
 * This file contains the declarations of classes, functions, and data structures
 * used in the web server implementation.
 */

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
#include "parsing/parsing.hpp"
#include "Cgi.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include <sys/types.h>
#include <netdb.h>

# define READ_BUFFER 1024
# define INACTIVITY_TIMEOUT 20

/***********************************************************************/
/*                           ┌─────────────┐                           */
/*                           │ SERVER PART │                           */
/*                           └─────────────┘                           */
/*             Beautifully managing server operations                  */
/***********************************************************************/

# define BACKLOG 30
typedef std::vector<int> int_v;

typedef enum HttpMethod {
    GET,
    POST,
    DELETE,
    NONE
} HttpMethod;

typedef struct clientInfo
{
    int clientSocket;               ///< Socket file descriptor for the client connection.
    HttpMethod method;              ///< HTTP method used in the request.
    bool isTransferChunked;         ///< Flag indicating if the request uses chunked transfer encoding.
    int contentLength;              ///< Content length of the request body.
    std::string request;            ///< HTTP request received from the client.
    std::string response;           ///< HTTP response to be sent to the client.
    size_t responseBytesSent;       ///< Tracking the number of bytes sent in the response.
    bool keepAlive;                 ///< Flag indicating if the connection should be kept alive.
    time_t lastActivity;            ///< Time of the last activity on the connection.
} clientInfo;

typedef std::map<int, clientInfo> clientInfoList;   ///< Map of client socket file descriptors to client information.
typedef clientInfoList::iterator clientInfoIt;                ///< Iterator for the clientInfoList.

/**
 * @brief Set up a server socket and start listening for incoming connections.
 *
 * @param port The port number to listen on.
 * @param backLog The maximum number of pending connections in the server's listen queue.
 * @return The server socket file descriptor.
 */
int  setupServer(int port, int backLog, const std::string &ip);

/**
 * @brief Accept a new client connection on the server socket.
 *
 * @param serverSocket The server socket file descriptor.
 * @return The client socket file descriptor.
 */
int  acceptNewConnection(int serverSocket);

/**
 * @brief Handle a client connection.
 *
 * @param clientInfo The client information.
 */
void handleConnection(std::vector<Server> &conFile, clientInfo &clientInfo);

/**
 * @brief Check for errors and handle them.
 *
 * @param sys The return value of a system call.
 * @param socketFD The socket file descriptor.
 */
void check_error(int sys, int socketFD);

/**
 * @class ServerManager
 * @brief Class for managing multiple server instances.
 */
class ServerManager {
    private:
        fd_set mainReadSet;             ///< Main read set for select().
        fd_set workingReadSet;          ///< Working read set for select().
        fd_set mainWriteSet;            ///< Main write set for select().
        fd_set workingWriteSet;         ///< Working write set for select().
        clientInfoList clientInfos;     ///< List of client information.
        int_v serverSockets;            ///< Vector of server socket file descriptors.
        int max_fds;                    ///< Maximum file descriptor value.
        std::vector<Server> &conFile;   ///< Configuration file.
    public:
        /**
         * @brief Default constructor for ServerManager.
         */
        ServerManager(std::vector<Server> &conFile);

        /**
         * @brief Set up multiple server instances.
         *
         * @param start The starting port number.
         * @param end The ending port number.
         */
        void setupServers();

        /**
         * @brief Run the server instances.
         */
        void run();

    private:
        /**
         * @brief Handle a new client connection.
         *
         * @param socket The client socket file descriptor.
         */
        void handleNewConnection(int socket);

        /**
         * @brief Handle incoming data from a client.
         *
         * @param socket The client socket file descriptor.
         */
        void handleIncomingData(int socket);

        /**
         * @brief Handles the sending of data over a socket.
         *
         * This function is responsible for sending data over the specified socket.
         *
         * @param socket The socket to send data over.
         */
        void handleSendingData(int socket);

        /**
         * @brief Check if a socket is ready for reading.
         *
         * @param socket The socket file descriptor.
         * @return True if the socket is ready for reading, false otherwise.
         */
        bool socketReadyForRead(int socket) const;

        /**
         * @brief Check if a socket is ready for writing.
         *
         * @param socket The socket file descriptor.
         * @return True if the socket is ready for writing, false otherwise.
         */
        bool socketReadyForWrite(int socket) const;

        /**
         * @brief Check if a socket is a server socket.
         *
         * @param socket The socket file descriptor.
         * @return True if the socket is a server socket, false otherwise.
         */
        bool isServerSocket(int socket) const;

        /**
         * @brief Create a new clientInfo object.
         *
         * @param clientSocket The client socket file descriptor.
         * @return The new clientInfo object.
         */
        clientInfo newClientInfo(int clientSocket) const;

        /**
         * @brief Update the clientInfo object with the received data.
         *
         * @param clientRequest The clientInfo object to update.
         * @param buffer The buffer containing the received data.
         * @param bytesRead The number of bytes read from the buffer.
         */
        void updateClientInfo(clientInfo &clientRequest, char buffer[], size_t const bytesRead);

        /**
         * @brief Detect the HTTP method used in the request.
         *
         * @param request The HTTP request.
         * @return The HTTP method.
         */
        HttpMethod detectMethod(const std::string &request) const;

        /**
         * @brief Check if the request is a complete message.
         *
         * @param clientRequest The clientInfo object.
         * @return True if the request is complete, false otherwise.
         */
        bool isCompleteMessage(clientInfo &clientRequest) const;

        /**
         * @brief Check if a string ends with another string.
         *
         * @param requestHeader The string to check.
         * @param requestEnd The string to compare with.
         * @return True if the string ends with the other string, false otherwise.
         */
        bool endsWith(std::string const &requestHeader, std::string const &requestEnd) const;

        /**
         * @brief Check if the length of the request body is valid.
         *
         * @param requestHeader The HTTP request header.
         * @param contentLength The content length of the request body.
         * @return True if the length is valid, false otherwise.
         */
        bool isRequestBodyLengthValid(std::string const &requestHeader, size_t contentLength) const;

        /**
         * @brief Filter out inactive connections.
        */
        void filterInactiveConnections();
};

#endif
