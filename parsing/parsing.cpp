/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-maar <ael-maar@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/04 14:50:47 by azaghlou          #+#    #+#             */
/*   Updated: 2024/02/06 19:15:20 by ael-maar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parsing.hpp"

bool global_var = false;

int count_char_appearing(std::string str, char c)
{
//  Description: Clear from its name, count the number of times a character appear in a string
    int count = 0;
    for (int x = 0; str[x]; x++)
    {
        if (str[x] == c)
            count++;
    }
    return (count);
}

std::string TrimeWhiteSpaces(std::string line)
{
//  Description: Clear from its name, take a string as parameter and return it without any sequenced spaces
    int x = 0;
    std::string str;
    while (line[x] && (line[x] == ' ' || line[x] == '\t'))
        x++;
    while (line[x])
    {
        if (line[x] && line[x] != ' ' && line[x] != '\t')
            str += line[x++];
        else
        {
            while (line[x] && (line[x] == ' ' || line[x] == '\t'))
                x++;
            str += ' ';
        }
    }
    if (str[str.size()-1] == ' ')
        str[str.size()-1] = '\0';
    return (str);
}

std::string FirstPart(std::string line)
{
//  Description: Take a string as parameter and return the first part of it before the first space or '='
    std::string str;

    std::string array[] = {"server_name", "port", "index", "auto_index", "error", "root", "method",
    "cgi", "[server]", "[location]", "name", "upload_path", "upload_enable", "body_size", "host", "return"};
    std::vector<std::string> vec(array, array + 16);

    for (int x = 0; line[x] && line[x] != ' ' && line[x] != '='; x++)
        str += line[x];
    for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end() && *it != str; ++it)
    {
        if (it + 1 == vec.end())
            throw std::string("Syntax Error");
    }
    return (str);
}

void count_of(std::string str, char c)
{
    int count = 0;
    for (int x = 0; str[x]; x++)
    {
        if (str[x] == c)
            count++;
    }
    if (count > 1 || count == 0)
        throw std::string("Syntax Error");
}

std::string SecondPart(std::string line)
{
//  Description: Take a string as parameter and return the second part of it after the first space or '='
    count_of(line, '=');
    int index = line.find('=');
    if (line[index] && line[index+1] && line[index+1] == ' ')
        index++;
    if (std::strncmp(&line[index+1], "", 1) == 0)
        throw std::string("Syntax Error");
    std::string str = &line[index+1];
    if (str.find(' ') != std::string::npos)
        throw std::string("Syntax Error");
    return (&line[index+1]);
}

int port_case(std::string value, bool port)
{
//  Description: I use this function while reading the port part of the conf file, it take's a string as parameter and return the port number
    char *letter;
    double un = std::strtod(value.c_str(), &letter);

    if (*letter || un < 0 || (un > 65535 && port))
        throw std::string("Syntax Error");
    return (un);
}

// unsigned int port_case(std::string value)
// {
// //  Description: I use this function while reading the port part of the conf file, it take's a string as parameter and return the port number
//     unsigned int un;
//     std::stringstream ss(value);

//     for (int x = 0; value[x]; x++)
//     {
//         if (!std::isdigit(value[x]))
//             global_var = true;
//     }
//     if (!(ss >> un))
//         return (global_var = true, 0);
//     return (un);
// }

bool CaseEqual(const std::string& str1, const std::string& str2)
{
//  Description: Check if two strings are equal without case sensitivity, like exAMplE and EXAMple
    if (str1.length() != str2.length())
        return false;

    for (size_t i = 0; i < str1.length(); i++)
    {
        if (std::tolower(str1[i]) != std::tolower(str2[i]))
            return false;
    }
    return (true);
}

bool    BoolCase(std::string value)
{
//  Description: I use this function while reading the auto_index part of conf file, it take's a string and check if it's true or false
//               and return it as a boolean, otherwisw it call the error function
    if (CaseEqual(value, "true"))
        return true;
    else if (CaseEqual(value, "false"))
        return false;
    else
        throw std::string("Syntax Error");
    return true;
}

std::vector<std::string> split(std::string str, char c)
{
    std::vector<std::string> vec;
    for (int x = 0; str[x]; x++)
    {
        std::string temp;
        while (str[x] && str[x] != c)
            temp += str[x++];
        temp.erase(std::remove(temp.begin(), temp.end(), ' '), temp.end());
        vec.push_back(temp);
    }
    return (vec);
}

std::pair<int, std::string>    errors_case(std::string line)
{
//  Description: I use this function while reading the errors part of the conf file, it take's a string as parameter and return a pair of the error code and the error path
    int error_code;
    std::string path;
    std::string html = ".html";
    std::string sec_part = SecondPart(line);
    std::vector<std::string> vec = split(sec_part, ':');
    char* letter;
    error_code = std::strtod(vec[0].c_str(), &letter);
    if (*letter || error_code < 100 || error_code > 599)
        throw std::string("Syntax Error");
    path = vec[1];
    if (&path[path.length()-5] != html || path.length() <= 5)
        global_var = true;
    return (std::make_pair(error_code, path));
}

Location    default_location(std::vector<Server>::iterator serv_it)
{
//  Description: This function is used to add a default location to the server if no location is specified
    Location L;

    L.set_location_name("/");
    L.set_root(serv_it->get_root());
    L.set_index(serv_it->get_index());
    L.set_body_size(serv_it->get_body_size());
    L.set_auto_index(serv_it->get_auto_index());
    L.set_methods("GET");
    L.set_methods("POST");
    L.set_methods("DELETE");
    return (L);

}

bool    checkOptionals(std::vector<Server> &vec)
{
//  Description: This function check if the optionals part of the conf file are filled or not, if not it call the error function
//               and also add the three methods GET, POST and DELETE if no methods are specified.
    for(std::vector<Server>::iterator it = vec.begin(); it != vec.end(); it++)
    {
        if (it->get_name().empty() || it->get_port_begin() == it->get_port_end() ||
            it->get_root().empty() || it->get_index().empty() || it->get_host().empty())
            return (true);
        if (it->get_locations_begin() == it->get_locations_end())
            it->locations.push_back(default_location(it));
        for(std::vector<Location>::iterator loc_it = it->get_locations_begin(); loc_it != it->get_locations_end(); loc_it++)
        {
            if (loc_it->get_location_name().empty() || loc_it->get_root().empty())
                return (true);
            if (loc_it->get_methods_begin() == loc_it->get_methods_end())
            {
                loc_it->set_methods("GET");
                loc_it->set_methods("POST");
                loc_it->set_methods("DELETE");
            }
        }
    }
    return (false);
}

void  home_checker(std::vector<Server> &vec)
{
//  Description: This function is used to check if the root path of the server is a valid path or not, if not it call the error function
    for(std::vector<Server>::iterator it = vec.begin(); it != vec.end(); it++)
    {
        bool Found = false;
        for (std::vector<Location>::iterator loc_it = it->get_locations_begin(); loc_it != it->get_locations_end(); loc_it++)
        {
            if (loc_it->get_location_name() == "/")
            {
                Found = true;
                break;
            }
        }
        if (!Found)
            it->locations.push_back(default_location(it));
    }
}

std::string host_case(std::string host)
{
    std::vector<std::string> nums = split(host, '.');
    if (nums.size() != 4)
        throw std::string("Syntax Error");
    for (std::vector<std::string>::iterator it = nums.begin(); it != nums.end(); it++)
    {
        char* letter;
        int num = std::strtod(it->c_str(), &letter);
        if (*letter || num < 0 || num > 255)
            throw std::string("Syntax Error");
    }
    return host;
}

std::vector<Server> ServerFill(std::string conf)
{
//  Description: This is the main function, it take the conf file as parameter and return a vector of server filled with all the data
    std::vector<Server> vec;
    std::string         line;
    std::string         var;
    std::string         block;
    std::ifstream       file(conf.c_str());
    std::vector<Server>::iterator serv_it = vec.begin();
    std::vector<Location>::iterator loc_it;
    int j = 0, k = 0, l = 0;

    if (!file.is_open())
    {
        std::cerr << "Error: file dont exist\n";
        exit(1);
    }
    while (1)
    {
        if (file.eof())
            break;
        std::getline(file, line, '\n');
        line = TrimeWhiteSpaces(line);
        if (line.empty())
            continue;
        if (line == "[server]")
        {
            Server S;
            vec.push_back(S);
            serv_it = vec.end() - 1;
            block = line;
        }
        else if (line == "[location]")
        {
            if (vec.size() == 0)
                return (throw std::string("Error"), vec);
            Location L(serv_it);
            serv_it->set_locations(L);
            loc_it = serv_it->locations.end() - 1;
            block = line;
            k = 0; j = 0; l = 0;
        }
        var = FirstPart(line);
        if (block == "[server]" && var == "server_name" && serv_it->get_name().empty())
            serv_it->set_name(SecondPart(line));
        else if (block == "[server]" && var == "port")
            serv_it->set_port(port_case(SecondPart(line), true));
        else if (block == "[server]" && var == "root" && serv_it->get_root().empty())
            serv_it->set_root(SecondPart(line));
        else if (block == "[server]" && var == "index" && serv_it->get_index().empty())
            serv_it->set_index(SecondPart(line));
        else if (block == "[server]" && var == "auto_index" && l++ == 0)
            serv_it->set_auto_index(BoolCase(SecondPart(line)));
        else if (block == "[server]" && var == "error")
            serv_it->set_errors(errors_case(line));
        else if (block == "[server]" && var == "body_size" && serv_it->get_body_size() == 1048576)
            serv_it->set_body_size(port_case(SecondPart(line), false));
        else if (block == "[server]" && var == "host" && serv_it->get_host().empty())
            serv_it->set_host(host_case(SecondPart(line)));

        else if (block == "[location]" && var == "name" && loc_it->get_location_name().empty())
            loc_it->set_location_name(SecondPart(line));
        else if (block == "[location]" && var == "root")
            loc_it->set_root(SecondPart(line));
        else if (block == "[location]" && var == "index")
            loc_it->set_index(SecondPart(line));
        else if (block == "[location]" && var == "upload_path" && loc_it->get_upload_path().empty())
            loc_it->set_upload_path(SecondPart(line));
        else if (block == "[location]" && var == "upload_enable" && j++ == 0)
            loc_it->set_upload_enable(BoolCase(SecondPart(line)));
        else if (block == "[location]" && var == "method")
            loc_it->set_methods(SecondPart(line));
        else if (block == "[location]" && var == "cgi" && loc_it->get_cgi_begin() == loc_it->get_cgi_end())
            loc_it->set_cgi(SecondPart(line));
        else if (block == "[location]" && var == "auto_index" && k++ == 0)
            loc_it->set_auto_index(BoolCase(SecondPart(line)));
        else if (block == "[location]" && var == "return" && loc_it->get_redirection().empty())
            loc_it->set_redirection(SecondPart(line));
        else if (block == "[location]" && var == "body_size")
            loc_it->set_body_size(port_case(SecondPart(line), false));

        else if (block != "[server]" && block != "[location]" && !block.empty())
            return (throw std::string("Syntax Error"), vec);
        if (global_var)
            return (throw std::string("Syntax Error"), vec);
        // line.clear();
    }
    if (checkOptionals(vec))
        throw std::string("Syntax Error");
    home_checker(vec);
    return (vec);
}

std::map<std::string, std::string>    &Location::get_cgi(){
//  Description: This function is a getter for the cgi map, it return the map
    return (this->cgi);
}

std::vector<Location>   &Server::get_locations(){
//  Description: This function is a getter for the locations vector, it return the vector
    return (this->locations);
}

std::map<int, std::string>                    &Server::get_errors(){
//  Description: This function is a getter for the errors map, it return the map
    return (this->errors);
}

void Server::set_host(std::string host){
//  Description: This function is a setter for the host attribute, it take a string as parameter and set the host attribute
    this->host = host;
}

std::string &Server::get_host(){
//  Description: This function is a getter for the host attribute, it return the host attribute
    return (this->host);
}
