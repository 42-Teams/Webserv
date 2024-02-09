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
			line.pop_back();
			if (line.back() == ';')
				line.pop_back();
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
		std::cout << "[WARNING] failed to open mime.types file using default mime types" << std::endl;
		mime_types["html"] = "text/html";
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

void remove_slash(std::string& str){
	if (str[0] == '/')
		str.erase(0,1);
	for (size_t i = 0; i < str.length(); i++){
		if (str[i] == '/' && str[i + 1] == '/')
			str.erase(i,1);
	}
	if (str.back() != '/' && str.rfind(".") != std::string::npos)
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
	remove_slash(uri);
	if (uri == "")
		return get_Home_location(locations);
	std::stringstream ss(uri);
	std::string tmp;
	std::string line;

	while (std::getline(ss,tmp,'/'))
	{
		line += "/" + tmp;
		for(size_t i = 0;i < locations.size();i++){
			if (locations[i].get_location_name() == line ){
				return locations[i];
			}
		}
	}
	throw std::out_of_range("404");
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
			throw (request.get_status().c_str());
		if ((int)request.get_raw_body().size() > server.get_body_size())
			throw std::runtime_error("413");
		Location location;
		location = find_location(server.get_locations(),request.get_path());
		std::string method = request.get_method();
		if (std::find(location.get_methods_begin(), location.get_methods_end(), method) == location.get_methods_end())
			throw std::runtime_error("405");
		else{
			if (method == "POST")
				Post(request, server,location);
			else if (method == "GET")
				Get(request, server,location);
			else if (method == "DELETE")
				Delete(request, server,location);
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

std::string getExtension(std::string type, std::map<std::string, std::string>& mime_types){
	for (std::map<std::string, std::string>::iterator it = mime_types.begin(); it != mime_types.end(); it++){
		if (it->second == type)
			return it->first;
	}
	return "";
}

void Response::upload_file(Request& request, Server& server, Location &location)
{
	(void)server;
	std::string ext = getExtension(request.get_headers()["Content-Type"],mime_types);
	std::string file_name = NameGenerator() + "." + ext;
	std::string file_path = location.get_upload_path() + "/" + file_name;
	std::string file_content = request.get_raw_body();
	std::ofstream file(file_path.c_str(), std::ios::binary);
	if (file.is_open()){
		file << file_content;
		file.close();
		_response = "HTTP/1.1 201 Created\r\ncontent-type: " + this->mime_types[getExtension(request.get_headers()["Content-Type"],mime_types)] + "\r\nContent-Disposition: attachment; filename=\"" + file_name + "\"\r\ncontent-length: " + to_string(file_content.length()) + "\r\n\r\n" + file_content;
		std::remove(file_path.c_str());
	}
	else
		throw std::runtime_error("500");
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
		if (!location.get_cgi().empty() && need_cgi(file_path, location.get_cgi())){
			Cgi cgi;
			cgi.execute_cgi(file_path, location.get_cgi(), request);
			_response = cgi.getResponse();
		}
		else{
			throw std::runtime_error("403");
		}
	}
	else
		throw std::runtime_error("404");
}

void Response::Post(Request& request, Server& server, Location &location)
{
	if (location.get_upload_enable())
		upload_file(request, server, location);
	else{
		non_upload(request, server, location);
	}
}

void Response::Get(Request& request, Server& server, Location &location)
{
	std::string path = request.get_path();
	std::string root = location.get_root();
	if (location.get_location_name() != "/")
		path = path.substr(location.get_location_name().length(), path.length());
	std::string file_path = root + path;
	struct stat file_stat;
	if (stat(file_path.c_str(), &file_stat) < 0)
		throw std::runtime_error("404");
	if (S_ISDIR(file_stat.st_mode))
		get_dir(request, server, location);
	else if (S_ISREG(file_stat.st_mode)){
		if (!location.get_cgi().empty() && need_cgi(file_path, location.get_cgi())){
			Cgi cgi;
			cgi.execute_cgi(file_path, location.get_cgi(), request);
			_response = cgi.getResponse();
		}
		else{
			std::ifstream file(file_path.c_str(), std::ios::binary);
			if (file.is_open()){
				std::string page((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
				file.close();
				std::string file_extension = file_path.substr(file_path.find_last_of(".") + 1);
				std::string content_type = mime_types[file_extension];
				if (content_type == "")
					content_type = "application/octet-stream";
				_response = "HTTP/1.1 200 OK\r\ncontent-type: " + content_type + "\r\ncontent-length: " + to_string(page.length()) + "\r\n\r\n" + page;
			}
			else
				throw std::runtime_error("404");
		}
	}
	else
		throw std::runtime_error("404");
}

void Response::Delete(Request& request, Server& server, Location &location)
{
	std::string path = request.get_path();
	std::string root = location.get_root();
	if (location.get_location_name() != "/")
		path = path.substr(location.get_location_name().length(), path.length());
	std::string file_path = root + path;
	struct stat file_stat;
	if (stat(file_path.c_str(), &file_stat) < 0)
		throw std::runtime_error("404");
	if (S_ISDIR(file_stat.st_mode))
		delete_dir(request, server,location);
	else if (S_ISREG(file_stat.st_mode)){
		if (!location.get_cgi().empty() && need_cgi(file_path, location.get_cgi())){
			Cgi cgi;
			cgi.execute_cgi(file_path, location.get_cgi(), request);
			_response = cgi.getResponse();
		}
		else{
			std::remove(file_path.c_str());
			_response = "HTTP/1.1 204 No Content\r\n\r\n";
		}
	}
	else
		throw std::runtime_error("404");
}

std::string Response::get_error_page(int code, Server& server)
{
	std::map<int, std::string> error_pages = server.get_errors();
	if (error_pages.find(code) != error_pages.end()){
		std::string path = server.get_root() +"/" + error_pages[code];
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
	_error_pages[400] = "HTTP/1.1 400 Bad Request\r\nContent-Length: 50\r\n\r\n<html><body><h1>400 Bad Request</h1></body></html>";
	_error_pages[401] = "HTTP/1.1 401 Unauthorized\r\nContent-Length: 51\r\n\r\n<html><body><h1>401 Unauthorized</h1></body></html>";
	_error_pages[403] = "HTTP/1.1 403 Forbidden\r\nContent-Length: 48\r\n\r\n<html><body><h1>403 Forbidden</h1></body></html>";
	_error_pages[404] = "HTTP/1.1 404 Not Found\r\nContent-Length: 48\r\n\r\n<html><body><h1>404 Not Found</h1></body></html>";
	_error_pages[405] = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 57\r\n\r\n<html><body><h1>405 Method Not Allowed</h1></body></html>";
	_error_pages[409] = "HTTP/1.1 409 Conflict\r\ncontent-type: text/html\r\nContent-Length: 47\r\n\r\n<html><body><h1>409 Conflict</h1></body></html>";
	_error_pages[413] = "HTTP/1.1 413 Payload Too Large\r\nContent-Length: 56\r\n\r\n<html><body><h1>413 Payload Too Large</h1></body></html>";
	_error_pages[500] = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 60\r\n\r\n<html><body><h1>500 Internal Server Error</h1></body></html>";
	_error_pages[501] = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 54\r\n\r\n<html><body><h1>501 Not Implemented</h1></body></html>";
	_error_pages[505] = "HTTP/1.1 505 HTTP Version Not Supported\r\nContent-Length: 65\r\n\r\n<html><body><h1>505 HTTP Version Not Supported</h1></body></html>";
	_error_pages[507] = "HTTP/1.1 507 Insufficient Storage\r\nContent-Length: 59\r\n\r\n<html><body><h1>507 Insufficient Storage</h1></body></html>";
}

void Response::init_error_messages(){
	_error_messages[400] = "Bad Request";
	_error_messages[401] = "Unauthorized";
	_error_messages[403] = "Forbidden";
	_error_messages[404] = "Not Found";
	_error_messages[405] = "Method Not Allowed";
	_error_messages[409] = "Conflict";
	_error_messages[413] = "Payload Too Large";
	_error_messages[500] = "Internal Server Error";
	_error_messages[501] = "Not Implemented";
	_error_messages[505] = "HTTP Version Not Supported";
	_error_messages[507] = "Insufficient Storage";
}

void Response::get_dir(Request& request, Server& server, Location& location)
{
	(void)server;
	std::string path = request.get_path();
	if (path[path.length() - 1] != '/'){
		path += "/";
		_response = "HTTP/1.1 301 Moved Permanently\r\nLocation: " + path + "\r\n\r\n";
		return ;
	}
	std::string root = location.get_root();
	path.replace(path.find(location.get_location_name()), location.get_location_name().length(), location.get_root());
	if (path[path.length() - 1] != '/')
		path += "/";
	std::string file_path = path;
	std::string index = location.get_index();
	if (index == ""){
		if (location.get_auto_index()){
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
						std::string file_path = root + path + file_name;
						page += "<li><a href=\"" + file_name + "\">" + file_name + "</a></li>";
					}
				}
				closedir(dir);
				page += "</ul></body></html>";
				_response = "HTTP/1.1 200 OK\r\ncontent-type: text/html\r\ncontent-length: " + to_string(page.size()) + "\r\n\r\n" + page;
			}
		}
		else
			throw std::runtime_error("403");
	}
	else{
		if (!location.get_cgi().empty() && need_cgi(index, location.get_cgi())){
			Cgi cgi;
			cgi.execute_cgi(file_path+index, location.get_cgi(), request);
			_response = cgi.getResponse();
		}
		else{
			std::string index_path = file_path + index;
			std::ifstream file(index_path.c_str(), std::ios::binary);
			if (file.is_open()){
				std::string page((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
				file.close();
				std::string file_extension = index_path.substr(index_path.find_last_of(".") + 1);
					std::string content_type = mime_types[file_extension];
				_response = "HTTP/1.1 200 OK\r\ncontent-type: "+content_type+"\r\ncontent-length: " + to_string(page.size()) + "\r\n\r\n" + page;
			}
			else
				throw std::runtime_error("404");
		}
	}
}

void Response::post_dir(Request& request, Server& server, Location& location)
{
	(void)server;
	std::string path = request.get_path();
	if (path[path.length() - 1] != '/'){
		path += "/";
		_response = "HTTP/1.1 301 Moved Permanently\r\nLocation: " + path + "\r\n\r\n";
		return ;
	}
	std::string root = location.get_root();
	path.replace(path.find(location.get_location_name()), location.get_location_name().length(), location.get_root());
	if (path[path.length() - 1] != '/')
		path += "/";
	std::string file_path = path;
	std::string index = location.get_index();
	if (index == ""){
		throw std::runtime_error("403");
	}
	else{
		if (!location.get_cgi().empty() && need_cgi(index, location.get_cgi())){
			Cgi cgi;
			cgi.execute_cgi(file_path+index, location.get_cgi(), request);
			_response = cgi.getResponse();
		}
		else{
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
				std::string dir_path = path + "/" + entry->d_name;
				delete_directory(dir_path);
			}
			else{
				std::string file_name = entry->d_name;
				std::string path_del = path + file_name;
				if (remove(path_del.c_str()) != 0){
					throw std::runtime_error("failed to delete file");
				}
			}

		}
	}
	closedir(dir);
	return ;
}

void Response::delete_dir(Request& request, Server& server, Location& location)
{
	(void)server;
	std::string path = request.get_path();
	if (path[path.length() - 1] != '/')
		throw std::runtime_error("409");
	std::string root = location.get_root();
	path.replace(path.find(location.get_location_name()), location.get_location_name().length(), location.get_root());
	if (path[path.length() - 1] != '/')
		path += "/";
	std::string file_path = path;
	std::string index = location.get_index();
	if (!location.get_cgi().empty() && need_cgi(index, location.get_cgi())){
		if (index == ""){
			throw std::runtime_error("403");
		Cgi cgi;
		cgi.execute_cgi(file_path+index, location.get_cgi(), request);
		_response = cgi.getResponse();
	}
	}
	else{
		try{
			delete_directory(file_path);
			_response = "HTTP/1.1 204 No Content\r\n\r\n";
		}
		catch(std::exception &e){
			throw std::runtime_error("404");
		}
	}
}

std::string Response::get_response() const
{
	return _response;
}
