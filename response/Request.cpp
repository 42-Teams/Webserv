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

bool Request::check_uri(){
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

void Request::check_request(){
    if (this->version != "HTTP/1.1")
        this->requesr_status = "505";
    else if (this->headers.find("Transfer-Encoding") != this->headers.end() && this->headers["Transfer-Encoding"] != "chunked")
        this->requesr_status = "501";
    else if (this->headers["Method"] == "POST" && this->headers.find("Transfer-Encoding") == this->headers.end() && this->headers.find("Content-Length") == this->headers.end())
        this->requesr_status = "400";
    else if (!check_uri())
        this->requesr_status = "400";
    else if (this->path.length() > 2048)
        this->requesr_status = "414";
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


void Request::creat_headers(std::string &str){
    std::stringstream ss(str);

    std::string firstLine;
    std::getline(ss,firstLine);
    std::stringstream fl(firstLine);
    std::string tmp;
    fl >> tmp;
    set_method(tmp);
    fl >> tmp;
    set_path(tmp);
    fl >> tmp;
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
        // if (key == "Content-Type"){
        //     if (value.find("boundary") != std::string::npos){
        //         this->b_boundary = value.substr(value.find("boundary"));
        //         value.erase(value.find(";"), value.length());
        //     }
        // }
        this->headers[key] = value;
    }
}

void Request::parse_request(std::string request){
    size_t pos = request.find("\r\n\r\n");
    if (pos == std::string::npos)
        return;
    std::string tmp = request.substr(0,pos);
    creat_headers(tmp);
    if (this->requesr_status != "OK")
        return;
    check_request();
    request.erase(0,pos + 4);
    if (request.length() > 0)
    {
        this->body = true;
        if (this->headers.find("Transfer-Encoding") != this->headers.end() && this->headers["Transfer-Encoding"] == "chunked")
            from_chuned_to_normal(request);
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

std::map<std::string, std::string> Request::get_uncode_form() const{
    return this->uncode_form;
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
