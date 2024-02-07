/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParsingUtils.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azaghlou <azaghlou@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/05 00:41:51 by azaghlou          #+#    #+#             */
/*   Updated: 2024/02/06 18:27:46 by azaghlou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parsing.hpp"

extern bool global_var;
int         count_char_appearing(std::string str, char c);
bool        CaseEqual(const std::string& str1, const std::string& str2);

//-----------------------------------Location-----------------------------------//

Location::Location()
{
    auto_index = false;
    upload_enable = false;
}

Location::~Location()
{}

void    Location::set_root(std::string Root)
{
    this->root = Root;
}

void    Location::set_index(std::string Index)
{
//  Description: location::index setter and check if the it ends with a .html if not call the error function
    std::string html = ".html";
    std::string php = ".php";
    if ((&Index[Index.length()-5] != html || Index.length() <= 5) &&
        (&Index[Index.length()-4] != php || Index.length() <= 4))
            global_var = true;
    this->index = Index;
}

void    Location::set_methods(std::string method)
{
//  Description: Location::methods setter and check if the method is GET, POST or DELETE otherwise it call the error function
    if (!CaseEqual(method, "GET") && !CaseEqual(method, "POST") && !CaseEqual(method, "DELETE"))
        global_var = true;
    this->methods.push_back(method);
}

int jump_to_next_char(std::string &str, int index, char c)
{
//  Description: A function that make set_cgi function more readable
    if (str[index] == ' ')
        index++;
    if (str[index] == c)
        index++;
    if (str[index] == ' ')
        index++;
    return (index);
}

void    Location::set_cgi(std::string CgiPath)
{
//  Description: locations::cgi setter and check if the CgiPath is valid otherwise it call the error function
    int index = 0;
    std::string part1;
    std::string part2;

    if (count_char_appearing(CgiPath, '|') + 1 != count_char_appearing(CgiPath, ':'))
        global_var = true;
    if (CgiPath.find("||") != std::string::npos || CgiPath.find("::") != std::string::npos || CgiPath.find("||") != std::string::npos || CgiPath.find(": :") != std::string::npos)
        global_var = true;
    while (1)
    {
        while (CgiPath[index] && CgiPath[index] != ':' && CgiPath[index] != ' ')
            part1 += CgiPath[index++];
        index = jump_to_next_char(CgiPath, index, ':');
        while (CgiPath[index] && CgiPath[index] != '|' && CgiPath[index] != ' ')
            part2 += CgiPath[index++];
        if (part1.empty() || part2.empty() || count_char_appearing(part2, ':') == 1)
            global_var = true;
        this->cgi.insert(std::make_pair(part1, part2));
        if (!CgiPath[index])
            break;
        part1.clear();
        part2.clear();
        // std::cout << "              is: " << CgiPath[index] << std::endl;
        index = jump_to_next_char(CgiPath, index, '|');
        // std::cout << "              is: " << CgiPath[index] << std::endl;
    }
}

void    Location::set_auto_index(bool AutoIndex)
{
    this->auto_index = AutoIndex;
}


void Location::set_location_name(const std::string& name)
{
    this->name = name;
}

void Location::set_upload_path(const std::string& upload_path)
{
    this->upload_path = upload_path;
}

void Location::set_upload_enable(bool upload_enable)
{
    this->upload_enable = upload_enable;
}

const std::string &Location::get_root() const
{
    return (this->root);
}

const std::string &Location::get_index() const
{
    return (this->index);
}

std::vector<std::string>::iterator    Location::get_methods_begin()
{
    return (this->methods.begin());
}

std::vector<std::string>::iterator    Location::get_methods_end()
{
    return (this->methods.end());
}

std::map<std::string, std::string>::iterator Location::get_cgi_begin()
{
    return (this->cgi.begin());
}

std::map<std::string, std::string>::iterator Location::get_cgi_end()
{
    return (this->cgi.end());
}

const bool        &Location::get_auto_index() const
{
    return (this->auto_index);
}

const std::string &Location::get_location_name() const
{
    return name;
}

const std::string &Location::get_upload_path() const
{
    return upload_path;
}

bool Location::get_upload_enable() const
{
    return upload_enable;
}

bool    Location::isEmpty(int flag)
{
//  Description: A function that check if the Location::methods container is empty and the flags are for checking or modifying its state
    static bool empty = true;

    if (flag == 0)
    {
        this->clear_cgi();
        this->clear_methods();
        empty = true;
    }
    else if (flag == 1)
        empty = false;
    return empty;
}

void    Location::clear_methods()
{
//  Description: A function that clear the Location::methods container
    this->methods.clear();
}

void    Location::clear_cgi()
{
//  Description: A function that clear the Location::cgi container
    this->cgi.clear();
}


//-----------------------------------Server-----------------------------------//

Server::Server()
{
    this->port = -1;             // -1 is a default value to know in case no port is set in the config file
    this->body_size = 1048576;   // 1048576 = 1 MB
}

Server::~Server()
{}

void    Server::set_name(std::string Name)
{
    this->name = Name;
}

void    Server::set_port(int Port)
{
    this->port = Port;
}

// void    Server::set_index(std::string Index)
// {
//     this->index = Index;
// }

// void    Server::set_auto_index(bool Auto_Index)
// {
//     this->auto_index = Auto_Index;
// }

void    Server::set_locations(Location L)
{
    this->locations.push_back(L);
}

void    Server::set_errors(std::pair<int, std::string> pair)
{
    this->errors.insert(pair);
}

const std::string &Server::get_name() const
{
    return name;
}

const int &Server::get_port() const
{
    return port;
}

void Server::set_body_size(int bodySize)
{
    this->body_size = bodySize;
}

const int &Server::get_body_size() const
{
    return this->body_size;
}

std::vector<Location>::iterator Server::get_locations_begin()
{
    return (locations.begin());
}

std::vector<Location>::iterator Server::get_locations_end()
{
    return (locations.end());
}

std::map<int, std::string>::iterator    Server::get_errors_end()
{
    return (this->errors.end());
}

std::map<int, std::string>::iterator    Server::get_errors_begin()
{
    return (this->errors.begin());
}

void Server::set_root(const std::string& root)
{
    this->root = root;
}

const   std::string &Server::get_root() const
{
    return root;
}
