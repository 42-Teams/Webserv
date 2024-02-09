#pragma once

#include <iostream>
#include <string>
#include <map>
#include <unistd.h>
#include "Request.hpp"
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

class Cgi
{
private:
	std::string response;
	std::map<std::string, std::string> paths;
public:
	void execute_cgi(std::string file, std::map<std::string,std::string> &cgis , Request &request);
	std::string getResponse() const;
	void setResponse(std::string response);
	Cgi();
	~Cgi();
};


