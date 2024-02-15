/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkhairou <mkhairou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/07 12:06:58 by mkhairou          #+#    #+#             */
/*   Updated: 2024/02/07 12:06:58 by mkhairou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../webserv.hpp"

Cgi::Cgi()
{
}

Cgi::~Cgi()
{
}

std::map<std::string, std::string> setup_env(std::string file, Request& request, std::string upload_path)
{
	std::map<std::string, std::string> env;
	env["REDIRECT_STATUS"] = "200";
	env["REQUEST_METHOD"] = request.get_method();
	env["QUERY_STRING"] = request.get_query();
	env["CONTENT_LENGTH"] = to_string(request.get_raw_body().size());
	env["CONTENT_TYPE"] = request.get_headers()["Content-Type"];
	env["SCRIPT_FILENAME"] = file;
	env["SCRIPT_NAME"] = file.substr(file.find_last_of('/') + 1);
	env["SERVER_SOFTWARE"] = "webserv";
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["UPLOAD_PATH"] = upload_path;
	return (env);
}

char **get_env(std::map<std::string, std::string> env)
{
	char **envp = new char*[env.size() + 1];
	int i = 0;
	for (std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); it++)
	{
		std::string tmp = it->first + "=" + it->second;
		envp[i] = new char[tmp.length() + 1];
		strcpy(envp[i], tmp.c_str());
		i++;
	}
	envp[i] = NULL;
	return (envp);
}

void setup_response(std::string response, std::string& return_response){
	size_t pos = response.find("\r\n\r\n");
	if (pos != std::string::npos)
	{
		std::string header = response.substr(0, pos);
		std::string body = response.substr(pos + 4);
		return_response = "HTTP/1.1 200 OK\r\n" + header + "\r\ncontent-length: " + to_string(body.size()) + "\r\n\r\n" + body;
	}
	else
	{
		return_response = "HTTP/1.1 200 OK\r\ncontent-type: text/html\r\ncontent-length: " + to_string(response.size()) + "\r\n\r\n" + response;
	}
}

void Cgi::execute_cgi(std::string file, std::map<std::string,std::string> &cgis , Request &request, std::string upload_path)
{
	std::string ext = file.substr(file.find_last_of('.') + 1);
	int status;
	std::string path = cgis[ext];
	if (path.empty())
		throw std::runtime_error("500");
	int fd[2];
	if (pipe(fd) == -1)
		throw std::runtime_error("500");
	pid_t pid = fork();
	if (pid == -1)
		throw std::runtime_error("500");
	char **argc = new char*[3];
	char **envp = get_env(setup_env(file, request, upload_path));

	if (pid == 0)
	{
		argc[0] = (char *)path.c_str();
		argc[1] = (char *)file.c_str();
		argc[2] = NULL;
		if (dup2(fd[1], STDOUT_FILENO) == -1)
			exit(127);
		close(fd[1]);
		if (dup2(fd[0], STDIN_FILENO) == -1)
			exit(127);
		close(fd[0]);
		execve(path.c_str(), argc, envp);
		exit(127);
	}
	else
	{
		write(fd[1], request.get_raw_body().c_str(), request.get_raw_body().size());
		close(fd[1]);
		waitpid(pid, &status, 0);
		delete[] argc;
		for (int i = 0; envp[i]; i++)
			delete[] envp[i];
		delete[] envp;
		if (WIFEXITED(status))
		{
			if (WEXITSTATUS(status) == 127){
				close(fd[0]);
				throw std::runtime_error("500");
			}else if (WEXITSTATUS(status) != 0){
				close(fd[0]);
				throw std::runtime_error("404");
			}
		}
		char buf[1024];
		std::string response;
		int len = 0;
		while ((len = read(fd[0], buf, 1024)) > 0)
		{
			buf[len] = '\0';
			response += buf;
		}
		setup_response(response, this->response);
		close(fd[0]);
	}
}

std::string Cgi::getResponse() const
{
	return (this->response);
}

void Cgi::setResponse(std::string response)
{
	this->response = response;
}
