/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkhairou <mkhairou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/14 13:11:43 by mkhairou          #+#    #+#             */
/*   Updated: 2023/12/24 16:42:22 by mkhairou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../webserv.hpp"

void init_mime_types(std::map<std::string, std::string>& mime_types){
	std::ifstream file_type("./response/mime.types");
	if (file_type.is_open()){
		std::string line;
		while (getline(file_type, line)){
			line.erase(line.size() -  1);
			if (line[line.size() -  1] == ';')
				line.erase(line.size() -  1);
			std::string type = line.substr(0, line.find("|"));
			std::string ext = line.substr(line.find("|") + 1, line.length()-1);
			std::stringstream ss(ext);
			std::string tmp;
			while (getline(ss, tmp, ' ')){
				mime_types[tmp] = type;
			}
		}
		file_type.close();
	}
	else{
		std::cerr << "\033[33m[WARNING] failed to open mime.types file using default mime types\033[0m" << std::endl;		mime_types["html"] = "text/html";
		mime_types["css"] = "text/css";
		mime_types["js"] = "text/javascript";
		mime_types["jpg"] = "image/jpeg";
		mime_types["jpeg"] = "image/jpeg";
		mime_types["png"] = "image/png";
		mime_types["gif"] = "image/gif";
		mime_types["ico"] = "image/x-icon";
		mime_types["woff"] = "font/woff";
		mime_types["ttf"] = "font/ttf";
		mime_types["otf"] = "font/otf";
		mime_types["txt"] = "text/plain";
		mime_types["mp4"] = "video/mp4";
		mime_types["mp3"] = "audio/mpeg";
	}
}

std::string run_cgi(std::string file_name, std::map<std::string,std::string> cgis, Request& request, Location& locations){
	Cgi cgi;
	cgi.execute_cgi(file_name, cgis, request, locations.get_upload_path());
	return cgi.getResponse();
}

void remove_slash(std::string& str){
	if (str[0] == '/')
		str.erase(0,1);
	for (size_t i = 0; i < str.length(); i++){
		if (str[i] == '/' && str[i + 1] == '/')
			str.erase(i,1);
	}
	if (str[str.size() -  1] != '/' && str.rfind(".") != std::string::npos)
	{
		str.erase(str.find_last_of("/") + 1, str.length());
	}
}

Location get_Home_location(std::vector<Location> locations){
	for (size_t i = 0; i < locations.size(); i++){
		if (locations[i].get_location_name() == "/")
			return locations[i];
	}
	throw std::out_of_range("404");
}


Location find_location(std::vector<Location> locations, std::string uri){
	while (!uri.empty())
	{
		for(size_t i = 0;i < locations.size();i++){
			if (locations[i].get_location_name() == uri ){
				return locations[i];
			}
		}
		uri = uri.substr(0, uri.find_last_of("/"));
	}
	return get_Home_location(locations);
}

bool need_cgi(std::string file, std::map<std::string, std::string>& cgi){
	std::string ext = file.substr(file.find_last_of(".") + 1);
	if (cgi.find(ext) != cgi.end())
		return true;
	return false;
}

Response::Response(Request& request, Server& server)
{
	init_mime_types(this->mime_types);
	init_error_pages();
	init_error_messages();
	try {
		if (request.get_status() != "OK")
			throw std::runtime_error(request.get_status().c_str());
		Location location;
		location = find_location(server.get_locations(),request.get_path());
		if ((int)request.get_raw_body().size() > location.get_body_size())
			throw std::runtime_error("413");
		std::string method = request.get_method();
		if (std::find(location.get_methods_begin(), location.get_methods_end(), method) == location.get_methods_end())
			throw std::runtime_error("405");
		else{
			if (method == "POST"){
				if (!location.get_redirection().empty()){
					this->_response = "HTTP/1.1 301 Moved Permanently\r\ncontent-length: 0\r\nLocation: " + location.get_redirection() + "\r\n\r\n";
					return;
				}
				Post(request, server,location);
			}
			else if (method == "GET"){
				if (!location.get_redirection().empty()){
					this->_response = "HTTP/1.1 301 Moved Permanently\r\ncontent-length: 0\r\nLocation: " + location.get_redirection() + "\r\n\r\n";
					return;
				}
				Get(request, server,location);
			}
			else if (method == "DELETE"){
				if (!location.get_redirection().empty()){
					this->_response = "HTTP/1.1 301 Moved Permanently\r\ncontent-length: 0\r\nLocation: " + location.get_redirection() + "\r\n\r\n";
					return;
				}
				Delete(request, server,location);
			}
			else
				throw std::runtime_error("405");
		}
	}catch(std::exception &e){
		_response = get_error_page(std::atoi(e.what()),server);
		return;
	}

}

Response::~Response()
{

}

std::string NameGenerator(){
	std::string name = "tmp";
	std::string alphabet = "abcdefghijklmnopqrstuvwxyz";
	for (size_t i = 0; i < 20; i++){
		name += alphabet[rand() % 26];
	}
	return name;
}

void check_fail_reason(std::string& file_path){
	if (access(file_path.c_str(), R_OK | W_OK) == -1)
		throw std::runtime_error("403");
	throw std::runtime_error("500");
}

std::string getExtension(std::string type, std::map<std::string, std::string>& mime_types){
	for (std::map<std::string, std::string>::iterator it = mime_types.begin(); it != mime_types.end(); it++){
		if (it->second == type)
			return it->first;
	}
	return "";
}

bool check_type(std::string content_type){
	if (content_type.empty())
		throw std::runtime_error("400");
	if (content_type != "application/x-www-form-urlencoded" && content_type.substr(0, 19) != "multipart/form-data;")
		return true;
	return false;
}

void save_file(std::string file_path, const std::string& file_content, std::string &_response){
	std::ofstream file(file_path.c_str(), std::ios::binary);
	if (file.is_open()){
		file << file_content;
		file.close();
		_response = "HTTP/1.1 201 Created\r\ncontent-length: 0\r\n\r\n";
	}
	else
		throw std::runtime_error("500");
}

void Response::upload_file(Request& request, Server& server, Location &location)
{
	(void)server;
	struct stat file_stat;
	if (stat(location.get_upload_path().c_str(), &file_stat) < 0)
		throw std::runtime_error("500");
	if (!S_ISDIR(file_stat.st_mode))
		throw std::runtime_error("500");
	struct statvfs buf;
	if (statvfs(location.get_upload_path().c_str(), &buf))
		throw std::runtime_error("500");
	std::vector<Form> form = request.get_form();
	if (form.size() == 0){
		if (check_type(request.get_headers()["Content-Type"])){
			if (buf.f_bfree * buf.f_frsize < request.get_raw_body().size())
				throw std::runtime_error("507");
			std::string file_path = location.get_upload_path() + "/" + NameGenerator() + "." + getExtension(request.get_headers()["Content-Type"],mime_types);
			save_file(file_path, request.get_raw_body(), _response);
		}
		return ;
	}
	for (size_t i = 0; i < form.size(); i++){
		if (form[i].is_file == true){
			std::string ext = getExtension(form[i].content_type,mime_types);
			std::string file_name = form[i].filename;
			std::string file_path = location.get_upload_path() + "/" + file_name;
			std::string file_content = form[i].value;
			if (file_content.size() > buf.f_bfree * buf.f_frsize)
				throw std::runtime_error("507");
			save_file(file_path, file_content, _response);
		}
	}
}

void Response::non_upload(Request& request, Server& server, Location &location)
{
	std::string path = request.get_path();
	std::string root = location.get_root();
	path = path.substr(location.get_location_name().length(), path.length());
	std::string file_path = root + path;
	struct stat file_stat;
	if (stat(file_path.c_str(), &file_stat) < 0)
		throw std::runtime_error("404");
	if (S_ISDIR(file_stat.st_mode))
		post_dir(request, server,location);
	else if (S_ISREG(file_stat.st_mode)){
		if (!location.get_cgi().empty() && need_cgi(file_path, location.get_cgi()))
			_response = run_cgi(file_path, location.get_cgi(), request, location);
		else{
			throw std::runtime_error("403");
		}
	}
	else
		throw std::runtime_error("404");
}

bool posted_to_file(std::string uri){

	struct stat file_stat;
	if (stat(uri.c_str(), &file_stat) < 0)
		return false;
	if (S_ISREG(file_stat.st_mode))
		return true;
	return false;
}

void Response::Post(Request& request, Server& server, Location &location)
{
	std::string path = request.get_path();
	std::string root = location.get_root();
	path = path.substr(location.get_location_name().length(), path.length());
	root = root + path;
	if (!posted_to_file(root) && location.get_upload_path() != "" && location.get_upload_enable())
		upload_file(request, server, location);
	else
		non_upload(request, server, location);
}

void Response::Get(Request& request, Server& server, Location &location)
{
	std::string path = request.get_path();
	std::string root = location.get_root();
	path = path.substr(location.get_location_name().length(), path.length());
	std::string file_path = root + path;
	struct stat file_stat;
	if (stat(file_path.c_str(), &file_stat) < 0)
		throw std::runtime_error("404");
	if (S_ISDIR(file_stat.st_mode))
		get_dir(request, server, location);
	else if (S_ISREG(file_stat.st_mode)){
		if (!location.get_cgi().empty() && need_cgi(file_path, location.get_cgi())){
			_response = run_cgi(file_path, location.get_cgi(), request, location);
		}
		else{
			if (file_path == location.get_root() && location.get_location_name() == "/")
				throw std::runtime_error("500");
			std::ifstream file(file_path.c_str(), std::ios::binary);
			if (file.is_open()){
				std::string page((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
				file.close();
				std::string file_extension = file_path.substr(file_path.find_last_of(".") + 1);
				std::string content_type = mime_types[file_extension];
				if (content_type == "")
					content_type = "application/octet-stream";
				_response = "HTTP/1.1 200 OK\r\nContent-Type: " + content_type + "\r\nContent-Length: " + to_string(page.length()) + "\r\n\r\n" + page;
			}
			else
				check_fail_reason(file_path);
		}
	}
	else
		throw std::runtime_error("404");
}

void Response::Delete(Request& request, Server& server, Location &location)
{
	std::string path = request.get_path();
	std::string root = location.get_root();
	path = path.substr(location.get_location_name().length(), path.length());
	std::string file_path = root + path;
	struct stat file_stat;
	if (stat(file_path.c_str(), &file_stat) < 0)
		throw std::runtime_error("404");
	if (S_ISDIR(file_stat.st_mode))
		delete_dir(request, server,location, file_path);
	else if (S_ISREG(file_stat.st_mode)){
		if (std::remove(file_path.c_str()) != 0)
			check_fail_reason(file_path);
		_response = "HTTP/1.1 204 No Content\r\n\r\n";
	}
	else
		throw std::runtime_error("404");
}

std::string Response::get_error_page(int code, Server& server)
{
	std::map<int, std::string> error_pages = server.get_errors();
	if (error_pages.find(code) != error_pages.end()){
		std::string path = server.get_root() + "/" + error_pages[code];
		std::ifstream file(path.c_str());
		if (file.is_open()){
			std::string line;
			std::string page;
			while (getline(file, line)){
				page += line;
			}
			file.close();
			return "HTTP/1.1 " + to_string(code) + " " + _error_messages[code] + "\r\n content-type: text/html\r\ncontent-length: " + to_string(page.size()) + "\r\n\r\n" + page;
		}
		else{
			return _error_pages[code];
		}
	}
	else{
		return _error_pages[code];
	}
}

void Response::init_error_pages()
{
	_error_pages[400] = "HTTP/1.1 400 Bad Request\r\ncontent-type: text/html\r\nContent-Length: 50\r\n\r\n<html><body><h1>400 Bad Request</h1></body></html>";
	_error_pages[401] = "HTTP/1.1 401 Unauthorized\r\ncontent-type: text/html\r\nContent-Length: 51\r\n\r\n<html><body><h1>401 Unauthorized</h1></body></html>";
	_error_pages[403] = "HTTP/1.1 403 Forbidden\r\ncontent-type: text/html\r\nContent-Length: 48\r\n\r\n<html><body><h1>403 Forbidden</h1></body></html>";
	_error_pages[404] = "HTTP/1.1 404 Not Found\r\ncontent-type: text/html\r\nContent-Length: 48\r\n\r\n<html><body><h1>404 Not Found</h1></body></html>";
	_error_pages[405] = "HTTP/1.1 405 Method Not Allowed\r\ncontent-type: text/html\r\nContent-Length: 57\r\n\r\n<html><body><h1>405 Method Not Allowed</h1></body></html>";
	_error_pages[409] = "HTTP/1.1 409 Conflict\r\ncontent-type: text/html\r\nContent-Length: 47\r\n\r\n<html><body><h1>409 Conflict</h1></body></html>";
	_error_pages[413] = "HTTP/1.1 413 Payload Too Large\r\ncontent-type: text/html\r\nContent-Length: 56\r\n\r\n<html><body><h1>413 Payload Too Large</h1></body></html>";
	_error_pages[414] = "HTTP/1.1 414 URI Too Long\r\ncontent-type: text/html\r\nContent-Length: 51\r\n\r\n<html><body><h1>414 URI Too Long</h1></body></html>";
	_error_pages[500] = "HTTP/1.1 500 Internal Server Error\r\ncontent-type: text/html\r\nContent-Length: 60\r\n\r\n<html><body><h1>500 Internal Server Error</h1></body></html>";
	_error_pages[501] = "HTTP/1.1 501 Not Implemented\r\ncontent-type: text/html\r\nContent-Length: 54\r\n\r\n<html><body><h1>501 Not Implemented</h1></body></html>";
	_error_pages[504] = "HTTP/1.1 504 Gateway Timeout\r\ncontent-type: text/html\r\nContent-Length: 54\r\n\r\n<html><body><h1>504 Gateway Timeout</h1></body></html>";
	_error_pages[505] = "HTTP/1.1 505 HTTP Version Not Supported\r\ncontent-type: text/html\r\nContent-Length: 65\r\n\r\n<html><body><h1>505 HTTP Version Not Supported</h1></body></html>";
	_error_pages[507] = "HTTP/1.1 507 Insufficient Storage\r\ncontent-type: text/html\r\nContent-Length: 59\r\n\r\n<html><body><h1>507 Insufficient Storage</h1></body></html>";
}

void Response::init_error_messages(){
	_error_messages[400] = "Bad Request";
	_error_messages[401] = "Unauthorized";
	_error_messages[403] = "Forbidden";
	_error_messages[404] = "Not Found";
	_error_messages[405] = "Method Not Allowed";
	_error_messages[409] = "Conflict";
	_error_messages[413] = "Payload Too Large";
	_error_messages[414] = "URI Too Long";
	_error_messages[500] = "Internal Server Error";
	_error_messages[501] = "Not Implemented";
	_error_messages[504] = "Gateway Timeout";
	_error_messages[505] = "HTTP Version Not Supported";
	_error_messages[507] = "Insufficient Storage";
}

void list_directory(std::string file_path, std::string& _response){
	DIR* dir = opendir(file_path.c_str());
	if (dir == NULL){
		throw std::runtime_error("404");
	}
	else{
		std::string page = "<html><body><ul>";
		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL){
			if (entry->d_name[0] != '.'){
				std::string file_name = entry->d_name;
				page += "<li><a href=\"" + file_name + "\">" + file_name + "</a></li>";
			}
		}
		closedir(dir);
		page += "</ul></body></html>";
		_response = "HTTP/1.1 200 OK\r\ncontent-type: text/html\r\ncontent-length: " + to_string(page.size()) + "\r\n\r\n" + page;
	}
}

void Response::get_dir(Request& request, Server& server, Location& location)
{
	(void)server;
	std::string path = request.get_path();
	if (path[path.length() - 1] != '/'){
		path += "/";
		_response = "HTTP/1.1 301 Moved Permanently\r\ncontent-length: 0\r\nLocation: " + path + "\r\n\r\n";
		return ;
	}
	std::string root = location.get_root();
	path = path.substr(location.get_location_name().length(), path.length());
	path = root + path;

	std::string file_path = path;
	std::string index = location.get_index();
	std::ifstream file((file_path + index).c_str(),std::ios::binary);
	if (file.is_open()){
		std::string page((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
		_response = "HTTP/1.1 200 OK\r\ncontent-type: text/html\r\ncontent-length: " + to_string(page.size()) + "\r\n\r\n" + page;
	}
	else if (location.get_auto_index())
		list_directory(file_path, _response);
	else
		throw std::runtime_error("403");
}

void Response::post_dir(Request& request, Server& server, Location& location)
{
	(void)server;
	std::string path = request.get_path();
	if (path[path.length() - 1] != '/'){
		path += "/";
		_response = "HTTP/1.1 301 Moved Permanently\r\ncontent-length: 0\r\nLocation: " + path + "\r\n\r\n";
		return ;
	}
	std::string root = location.get_root();
	path.replace(path.find(location.get_location_name()), location.get_location_name().length(), location.get_root());
	if (path[path.length() - 1] != '/')
		path += "/";
	std::string file_path = server.get_root() + path;
	std::string index = location.get_index();
	if (index == ""){
		throw std::runtime_error("403");
	}
	else{
		if (!location.get_cgi().empty() && need_cgi(index, location.get_cgi())){
			_response = run_cgi(file_path + index, location.get_cgi(), request, location);
		}
		else{
			if (location.get_auto_index())
				list_directory(file_path, _response);
			else
				throw std::runtime_error("403");
		}
	}
}

void delete_directory(std::string path){
	DIR* dir = opendir(path.c_str());
	if (dir == NULL){
		throw std::runtime_error("failed to open directory");
	}
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL){
		if (entry->d_name[0] != '.'){
			if (entry->d_type == DT_DIR){
				std::string dir_path = path + "/" + entry->d_name + "/";
				delete_directory(dir_path);
			}
			else{
				std::string file_name = entry->d_name;
				std::string path_del = path + file_name;
				if (std::remove(path_del.c_str()) != 0){
					throw std::runtime_error("failed to delete file");
				}
			}

		}
	}
	closedir(dir);
	return ;
}

void Response::delete_dir(Request& request, Server& server, Location& location, std::string path)
{
	(void)server;
	(void)request;
	if (path[path.length() - 1] != '/')
		throw std::runtime_error("409");
	std::string root = location.get_root();
	std::string file_path = path;
	std::string index = location.get_index();
	try{
		delete_directory(file_path);
		_response = "HTTP/1.1 204 No Content\r\n\r\n";
	}
	catch(std::exception &e){
		check_fail_reason(file_path);
	}
}

std::string Response::get_response() const
{
	return _response;
}
