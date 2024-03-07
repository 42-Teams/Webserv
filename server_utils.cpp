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

std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

uint32_t ipToNetworkByteOrder(const std::string& ip) {
    std::vector<std::string> parts = split(ip, '.');
    uint32_t ipInNetworkByteOrder = 0;
    for (int i = 0; i < 4; ++i) {
        ipInNetworkByteOrder |= (std::atoi(parts[i].c_str()) << (24 - (8 * i)));
    }
    return ipInNetworkByteOrder;
}

int setupServer(int port, int backLog, const std::string &ip)
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    check_error(serverSocket, serverSocket);

    check_error(fcntl(serverSocket, F_SETFL, O_NONBLOCK, FD_CLOEXEC), serverSocket);
    int opt = 1;
    check_error(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)), serverSocket);
    sockaddr_in serverAddress = {};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(ipToNetworkByteOrder(ip));
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

bool has_port(std::vector<int> ports, int host_port)
{
    for (size_t i = 0; i < ports.size(); i++)
    {
        if (ports[i] == host_port)
            return true;
    }
    return false;
}

std::string ip_to_string(int ip)
{
   std::string result;
   int first_b;
   int second_b;
   int third_b;
   int last_b;
   first_b = ip >> 24 & 255;
   second_b = ip >> 16 & 255;
   third_b = ip >> 8 & 255;
   last_b = ip & 255;
   result = to_string(first_b) + "." + to_string(second_b) + "." + to_string(third_b) + "." +to_string(last_b);
   return result;
}

std::string get_host_ip(std::string host)
{
    struct addrinfo hints, *res, *p;
    int status;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    status  = getaddrinfo(host.c_str(), NULL, &hints, &res);
    if (status != 0)
        return "";
    int result = -1;
    for (p = res; p != NULL; p = p->ai_next)
    {
        result = ntohl(((struct sockaddr_in *)p->ai_addr)->sin_addr.s_addr);
       if (result != -1)
           break;
    }
    freeaddrinfo(res);
    return ip_to_string(result);
}

Server find_by_ip(std::vector<Server> &conFile, std::string ip, int port)
{
    for (size_t i = 0; i < conFile.size(); i++)
    {
        if (conFile[i].get_host() == ip && has_port(conFile[i].get_port(), port))
            return conFile[i];
    }
    return conFile[0];
}

Server findServer(std::vector<Server> &conFile, Request &request)
{
    std::string host = request.get_headers()["Host"];
    std::string port = host.substr(host.find(":") + 1);
    host = host.substr(0, host.find(":"));
    if (port == host)
        port = "80";
    std::vector<Server> servers;
    for (size_t i = 0; i < conFile.size(); i++)
    {
        if (conFile[i].get_name() == host)
            servers.push_back(conFile[i]);
    }
    host = get_host_ip(host);
    if (servers.size() == 1)
        return servers[0];
    else if (servers.size() > 1)
    {
        for (size_t i = 0; i < servers.size(); i++)
        {
            if (servers[i].get_host() == host && has_port(servers[i].get_port(), std::atoi(port.c_str())))
            {
                return servers[i];
            }
        }
    }
    return find_by_ip(conFile, host, std::atoi(port.c_str()));
}

void handleConnection(std::vector<Server> &conFile, clientInfo &clientInfo)
{
    Request request(clientInfo.request);
    std::string connection_status = request.get_headers()["Connection"];
    clientInfo.keepAlive = false;
    if (connection_status == "keep-alive")
        clientInfo.keepAlive = true;
    Server server = findServer(conFile, request);
    Response response(request,server);
    clientInfo.response = response.get_response();
}
