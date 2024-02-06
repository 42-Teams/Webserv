/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: azaghlou <azaghlou@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/04 14:50:47 by azaghlou          #+#    #+#             */
/*   Updated: 2024/02/06 18:26:17 by azaghlou         ###   ########.fr       */
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
    "cgi", "[server]", "[location]", "name", "upload_path", "upload_enable", "body_size", ""};
    std::vector<std::string> vec(array, array + 15);
    
    for (int x = 0; line[x] && line[x] != ' ' && line[x] != '='; x++)
        str += line[x];
    for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end() && *it != str; ++it)
    {
        if (it + 1 == vec.end())
            global_var = true;
    }
    return (str);
}

std::string SecondPart(std::string line)
{
//  Description: Take a string as parameter and return the second part of it after the first space or '='
    int index = line.find('=');
    if (line[index] && line[index+1] && line[index+1] == ' ')
        index++;
    if (std::strncmp(&line[index+1], "", 1) == 0)
        global_var = true;
    return (&line[index+1]);
}

unsigned int port_case(std::string value)
{
//  Description: I use this function while reading the port part of the conf file, it take's a string as parameter and return the port number
    unsigned int un;
    std::stringstream ss(value);

    for (int x = 0; value[x]; x++)
    {
        if (!std::isdigit(value[x]))
        global_var = true;
    }
    if (!(ss >> un))
        return (global_var = true, 0);
    return (un);
}

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

bool    return_bool(std::string value)
{
//  Description: I use this function while reading the auto_index part of conf file, it take's a string and check if it's true or false
//               and return it as a boolean, otherwisw it call the error function
    if (CaseEqual(value, "true"))
        return true;
    else if (CaseEqual(value, "false"))
        return false;
    else
        global_var = true;
    return true;
}

std::pair<int, std::string>    errors_case(std::string line)
{
//  Description: I use this function while reading the errors part of the conf file, it take's a string as parameter and return a pair of the error code and the error path
    int x = 0;
    int error_code;
    std::string path;
    std::string html = ".html";
    std::string sec_part = SecondPart(line);
    std::stringstream ss(sec_part);

    if (!(ss >> error_code))
        global_var = true;
    while(sec_part[x] && (std::isdigit(sec_part[x]) || sec_part[x] == ' '))
        x++;
    path = &sec_part[x];
    if (&path[path.length()-5] != html || path.length() <= 5)
        global_var = true;
    return (std::make_pair(error_code, path));
}

void    print_all(std::vector<Server> &vec)
{
//  Description: This function is just for testing, it print all the data that we have in the vector of server
    for(std::vector<Server>::iterator serv_it = vec.begin(); serv_it != vec.end(); serv_it++)
    {
        std::vector<Location>::iterator loct_it = serv_it->get_locations_begin();
        std::vector<Location>::iterator loct_end = serv_it->get_locations_end();
        std::cout << "[server]\n";
        std::cout << "server_name = " << serv_it->get_name() << std::endl;
        std::cout << "port = " << serv_it->get_port() << std::endl;
        std::cout << "root = " << serv_it->get_root() << std::endl;
        for(std::map<int, std::string>::iterator err = serv_it->get_errors_begin(); err != serv_it->get_errors_end(); err++)
            std::cout << "error = " << err->first << " " << err->second << std::endl;
        while(loct_it != loct_end)
        {
            std::cout << "\n[location]\n";
            std::cout << "location_name = " << loct_it->get_location_name() << std::endl;
            std::cout << "root = " << loct_it->get_root() << std::endl;
            if (!loct_it->get_index().empty())
                std::cout << "index = " <<  loct_it->get_index()  << std::endl;
            if (!loct_it->get_upload_path().empty())
                std::cout << "upload_path = " << loct_it->get_upload_path() << std::endl;
            std::cout << "upload_enable = " << loct_it->get_upload_enable() << std::endl;

            for(std::vector<std::string>::iterator it = loct_it->get_methods_begin(); it != loct_it->get_methods_end(); it++)
                std::cout << "method = " << *it << std::endl;
            for(std::map<std::string, std::string>::iterator it = loct_it->get_cgi_begin(); it != loct_it->get_cgi_end(); it++)
                std::cout << "cgi = " << it->first << " : " << it->second << std::endl;
            std::cout << "auto_index = " << loct_it->get_auto_index() << std::endl;
            if (loct_it->get_body_size() != -1)
                std::cout << "body_size = " << loct_it->get_body_size() << std::endl;
            loct_it++;
        }
        std::cout << "\n";
    }
}

bool    checkOptionals(std::vector<Server> &vec)
{
//  Description: This function check if the optionals part of the conf file are filled or not, if not it call the error function
//               and also add the three methods GET, POST and DELETE if no methods are specified.
    for(std::vector<Server>::iterator it = vec.begin(); it != vec.end(); it++)
    {
        if (it->get_name().empty() || it->get_port() == -1 || it->get_root().empty())
            return (true);
        std::vector<Location>::iterator loc_it = it->get_locations_begin();
        for(; loc_it != it->get_locations_end(); loc_it++)
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

std::vector<Server> ServerFill(std::string conf)
{
//  Description: This is the main function, it take the conf file as parameter and return a vector of server filled with all the data
    std::vector<Server> vec;
    std::string         line;
    std::string         var;
    std::string         block;
    std::ifstream       file(conf);
    std::vector<Server>::iterator serv_it = vec.begin();
    std::vector<Location>::iterator loc_it;
    int j = 0, k = 0;

    if (!file.is_open())
        std::cerr << "Error: file dont exist\n";
    while (1)
    {
        if (file.eof())
            break;
        std::getline(file, line);
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
            Location L;
            serv_it->set_locations(L);
            loc_it = serv_it->locations.end() - 1;
            block = line;
            k = 0; j = 0;
        }
        std::cout << "The line: " << line << std::endl;
        var = FirstPart(line);
        if (block == "[server]" && var == "server_name" && serv_it->get_name().empty())
            serv_it->set_name(SecondPart(line));
        else if (block == "[server]" && var == "port" && serv_it->get_port() == -1)
            serv_it->set_port(port_case(SecondPart(line)));
        else if (block == "[server]" && var == "root" && serv_it->get_root().empty())
            serv_it->set_root(SecondPart(line));
        else if (block == "[server]" && var == "error")
            serv_it->set_errors(errors_case(line));

        else if (block == "[location]" && var == "name" && loc_it->get_location_name().empty())
            loc_it->set_location_name(SecondPart(line));
        else if (block == "[location]" && var == "root" && loc_it->get_root().empty())
            loc_it->set_root(SecondPart(line));
        else if (block == "[location]" && var == "index" && loc_it->get_index().empty())
            loc_it->set_index(SecondPart(line));
        else if (block == "[location]" && var == "upload_path" && loc_it->get_upload_path().empty())
            loc_it->set_upload_path(SecondPart(line));
        else if (block == "[location]" && var == "upload_enable" && j++ == 0)
            loc_it->set_upload_enable(return_bool(SecondPart(line)));
        else if (block == "[location]" && var == "method")
            loc_it->set_methods(SecondPart(line));
        else if (block == "[location]" && var == "cgi" && loc_it->get_cgi_begin() == loc_it->get_cgi_end())
            loc_it->set_cgi(SecondPart(line));
        else if (block == "[location]" && var == "auto_index" && k++ == 0)
            loc_it->set_auto_index(return_bool(SecondPart(line)));
        else if (block == "[location]" && var == "body_size" && loc_it->get_body_size() == 1048576)
            loc_it->set_body_size(port_case(SecondPart(line)));
        else if (line != "[server]" && line != "[location]")
            return (throw std::string("Error"), vec);
        if (global_var == true)
            return (throw std::string("Error"), vec);
    }
    if (checkOptionals(vec))
        throw std::string("Error");
    return (vec);
}
