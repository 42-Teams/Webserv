/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkhairou <mkhairou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/02 13:21:13 by mkhairou          #+#    #+#             */
/*   Updated: 2023/12/12 15:23:42 by mkhairou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../webserv.hpp"

Request::Request(std::string request)
{
    this->requesr_status = "OK";
    parse_request(request);
}

Request::~Request()
{
}

std::vector<std::string> split(std::string str, std::string delim){
    std::vector<std::string> vec;
    size_t pos = 0;
    std::string token;
    while ((pos = str.find(delim)) != std::string::npos) {
        token = str.substr(0, pos);
        vec.push_back(token);
        str.erase(0, pos + delim.length());
    }
    vec.push_back(str);
    return vec;
}

void get_name(std::string &str, Form &form){
    std::string tmp;
    std::stringstream ss(str);
    std::getline(ss,tmp,'\n');
    if (tmp.back() == '\r')
        tmp.erase(tmp.length() - 1);
    std::vector<std::string> vec = split(tmp, "; ");
    if (vec.size() > 2){
        form.is_file = true;
        std::string filename = split(vec[2], "=\"")[1];
        filename.erase(filename.length() - 1);
        form.filename = filename;
        std::string name = split(vec[1], "=\"")[1];
        name.erase(name.length() - 1);
        form.name = name;
    }
    else{
        std::string name = split(vec[1], "=\"")[1];
        name.erase(name.length() - 1);
        form.name = name;
    }
    vec.clear();;
}

std::string edit_last_line(std::string str){
    size_t pos1 = str.find_first_of("\r\n");
    if (pos1 == std::string::npos)
        return str;
    str.erase(pos1, 2);
    size_t pos2 = str.find_last_of("\r\n");
    if (pos2 == std::string::npos)
        return str;
    str.erase(pos2, 2);
    return str;
}

void create_input(std::string &str, std::vector<Form>& form){
    Form tmp;
    tmp.is_file = false;
    std::string to_remove;
    get_name(str, tmp);
    std::string tmp2;
    std::stringstream ss(str);
    std::getline(ss,tmp2,'\n');
    to_remove += tmp2 + '\n';
    if (tmp.is_file){
        std::getline(ss,tmp2,'\n');
        to_remove += tmp2 + '\n';
        if (tmp2.back() == '\r')
            tmp2.erase(tmp2.length() - 1);
        tmp2.erase(0,tmp2.find(":") + 2);
        tmp.content_type = tmp2;
    }
    std::getline(ss,tmp2,'\n');
    to_remove += tmp2 + '\n';
    str.erase(0, to_remove.length());
    if (str.back() == '\n')
        str.erase(str.length() - 1);
    tmp.value = str;
    form.push_back(tmp);
}

void Request::parse_form(std::string form, std::string boundary){
    std::vector<std::string> vec = split(form, "--"+boundary);

    for (size_t i = 0; i <  vec.size(); i++)
    {
        vec[i].erase(0, 2);
        if (vec[i]=="\r\n" || vec[i]=="\r" || vec[i]=="\n" || vec[i]=="" || vec[i]=="--" || vec[i]=="--\r\n" || vec[i]=="--\r" || vec[i]=="--\n")
        {
            vec.erase(vec.begin() + i);
            i--;
        }
    }
    for (size_t i = 0; i <  vec.size(); i++)
        create_input(vec[i], this->form);
}


bool Request::check_uri(){
    if (this->path.empty())
        return false;
    std::string allowed_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%";
    for (size_t i = 0; i < this->path.length(); i++)
    {
        if (allowed_chars.find(this->path[i]) == std::string::npos)
            return false;
    }
    if (this->path.find("?") != std::string::npos){
        this->query = this->path.substr(this->path.find("?") + 1);
        this->path.erase(this->path.find("?"));
    }
    return true;
}

bool check_method(std::string method){
    if (method == "GET" || method == "HEAD" || method == "POST" || method == "PUT" || method == "DELETE" || method == "CONNECT" || method == "OPTIONS" || method == "TRACE")
        return true;
    return false;
}

void Request::check_request(){
    if (this->version == "" || this->method == "" || this->path == "" || this->headers.empty())
        this->requesr_status = "400";
    else if (this->version != "HTTP/1.1")
        this->requesr_status = "505";
    else if (!check_method(this->method))
        this->requesr_status = "400";
    else if (this->headers.find("Host") == this->headers.end())
        this->requesr_status = "400";
    else if (this->headers.find("Transfer-Encoding") != this->headers.end() && this->headers["Transfer-Encoding"] != "chunked")
        this->requesr_status = "501";
    else if (this->method == "POST" && this->headers.find("Transfer-Encoding") == this->headers.end() && this->headers.find("Content-Length") == this->headers.end())
        this->requesr_status = "400";
    else if (!check_uri())
        this->requesr_status = "400";
    else if (this->path.length() > 2048)
        this->requesr_status = "414";
    for (std::map<std::string, std::string>::iterator it = this->headers.begin(); it != this->headers.end(); it++)
    {
        if (it->second == "")
            this->requesr_status = "400";
    }
}

void from_chuned_to_normal(std::string& body){
    std::string tmp_body;
    int chunk_size;
    std::stringstream ss(body);
    while (1)
    {
        std::string place_holder;
        std::getline(ss,place_holder,'\r');
        ss.get();
        chunk_size = static_cast<int>(std::strtol(place_holder.c_str(),NULL,16));
        if (chunk_size == 0)
            break;
        std::string new_string(chunk_size,'\0');
        ss.read(&new_string[0],chunk_size);
        ss.get();
        ss.get();
        tmp_body += new_string;
    }
    body = tmp_body;
}

void remove_duplicate_slash(std::string &str){
    size_t pos = 0;
    while ((pos = str.find("//",pos)) != std::string::npos)
        str.erase(pos,1);
}

void Request::creat_headers(std::string &str){
    std::stringstream ss(str);

    std::string firstLine;
    std::getline(ss,firstLine);
    std::stringstream fl(firstLine);
    std::string tmp;
    fl >> tmp;
    if (fl.eof())
        return;
    set_method(tmp);
    fl >> tmp;
    if (fl.eof())
        return;
    set_path(tmp);
    fl >> tmp;
    if (fl.eof())
        return;
    set_version(tmp);
    while (std::getline(ss,tmp,'\n'))
    {
        if (tmp == "\r")
            break;
        std::stringstream line(tmp);
        std::string key;
        std::string value;
        std::getline(line,key,':');
        std::getline(line,value);
        if (value != "")
            value.erase(0,1);
        if (value[value.length() - 1] == '\r')
            value.erase(value.length() - 1);
        this->headers[key] = value;
    }
}

void fix_space_in_path(std::string &path){
    size_t pos = 0;
    while ((pos = path.find("%20",pos)) != std::string::npos)
    {
        path.replace(pos,3," ");
        pos++;
    }
}

void Request::parse_request(std::string request){
    size_t pos = request.find("\r\n\r\n");
    if (pos == std::string::npos)
        return;
    std::string tmp = request.substr(0,pos);
    creat_headers(tmp);
    check_request();
    if (this->requesr_status != "OK")
        return;
    fix_space_in_path(this->path);
    request.erase(0,pos + 4);

    if (request.length() > 0)
    {
        this->body = true;
        if (this->headers.find("Transfer-Encoding") != this->headers.end() && this->headers["Transfer-Encoding"] == "chunked")
            from_chuned_to_normal(request);
        std::string content_type;
        if (this->headers.find("Content-Type") != this->headers.end())
            content_type = this->headers["Content-Type"];
        if (!content_type.empty() && "multipart/form-data" == content_type.substr(0,19))
        {
            std::string boundary = content_type.substr(content_type.find("boundary=") + 9);
            parse_form(request, boundary);
        }
        this->raw_body = request;
    }
    else
        this->body = false;

}

void Request::set_method(std::string method){
    this->method = method;
}

void Request::set_path(std::string path){
    this->path = path;
}

void Request::set_version(std::string version){
    this->version = version;
}

void Request::set_host(std::string host){
    // this->host = host;
    (void)host;
}

std::string Request::get_method() const{
    return this->method;
}

std::string Request::get_path() const{
    return this->path;
}

std::string Request::get_version() const{
    return this->version;
}

std::string Request::get_host() const{
    return this->headers.at("Host");
}

std::map<std::string, std::string> Request::get_headers() const{
    return this->headers;
}

std::string Request::get_raw_body() const{
    return this->raw_body;
}

std::string Request::get_binary_body() const{
    return this->binary_body;
}

std::string Request::get_status() const{
    return this->requesr_status;
}

std::string Request::get_query() const{
    return this->query;
}

std::string to_string(int num){
    std::stringstream ss;
    ss << num;
    return ss.str();
}

std::vector<Form>  Request::get_form() const{
    return this->form;
}
