/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkhairou <mkhairou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/14 13:09:18 by mkhairou          #+#    #+#             */
/*   Updated: 2023/12/17 15:06:32 by mkhairou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <map>

class Response
{
private:
	std::string _response;
	std::map<int, std::string> _error_messages;
	std::map <int, std::string> _error_pages;
	void Post(Request& request, Server& server,Location &location);
	void Get(Request& request, Server& server, Location &location);
	void Delete(Request& request, Server& server, Location& location);
	void init_error_pages();
	void init_error_messages();
	std::string get_error_page(int code, Server& server);
	void get_dir(Request& request, Server& server, Location& location);
	void upload_file(Request& request, Server& server, Location& location);
	void non_upload(Request& request, Server& server, Location& location);
	void delete_dir(Request& request, Server& server, Location& location);
	void post_dir(Request& request, Server& server, Location& location);
	std::map<std::string, std::string> mime_types;
public:
	Response(Request& request, Server& server);
	std::string get_response() const;
	~Response();
};
