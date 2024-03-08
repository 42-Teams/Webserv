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

std::string get_upload_path(std::string file, std::string upload_path)
{
	struct  stat st;
	if (upload_path.empty())
		return (file.substr(0, file.find_last_of('/')));
	else if (stat(upload_path.c_str(), &st) == -1)
		return (file.substr(0, file.find_last_of('/')));
	else
		return (upload_path);
}

std::map<std::string, std::string> setup_env(std::string file, Request& request, std::string upload_path)
{
	std::map<std::string, std::string> env;
	env["REDIRECT_STATUS"] = "200";
	env["REQUEST_METHOD"] = request.get_method();
	env["QUERY_STRING"] = request.get_query();
	env["CONTENT_LENGTH"] = request.get_headers()["Content-Length"];
	env["CONTENT_TYPE"] = request.get_headers()["Content-Type"];
	env["PATH_INFO"] = file;
	env["PATH_TRANSLATED"] = "./" + file.substr(file.find_last_of('/') + 1);
	env["SERVER_SOFTWARE"] = "webserv";
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["SERVER_PROTOCOL"] = request.get_version();
	env["REMOTE_HOST"] = request.get_headers()["Host"];
	env["UPLOAD_PATH"] = get_upload_path(file, upload_path);
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


void parse_headers(std::string header, std::map<std::string, std::string>& headers)
{
    size_t pos = 0;
    while ((pos = header.find("\r\n")) != std::string::npos)
    {
        std::string line = header.substr(0, pos);
        header.erase(0, pos + 2);
        size_t pos = line.find(":");
        if (pos != std::string::npos)
        {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            headers[key] = value;
        }
    }
}

void setup_response(std::string& response, std::string& return_response){
    size_t pos = response.find("\r\n\r\n");
    std::string responseLine;
	if (pos != std::string::npos)
	{
        std::map<std::string, std::string> headers;
		std::string header = response.substr(0, pos + 2);
        parse_headers(header, headers);
		std::string body = response.substr(pos + 4);
        if (headers["Content-type"].empty() && headers["location"].empty())
           throw std::runtime_error("500");
        if (headers["Status"].empty())
            responseLine = "200 OK";
        else
            responseLine = headers["Status"];
		return_response = "HTTP/1.1 " + responseLine + "\r\n" + header;
        if (headers["Content-Length"].empty())
            return_response += "Content-Length: " + to_string(body.size()) + "\r\n";
        return_response += "\r\n" + body;
	}
	else
        throw std::runtime_error("500");
}

void close_pipe(int input[2], int output[2])
{
    close(input[0]);
    close(input[1]);
    close(output[0]);
    close(output[1]);
}

void Cgi::execute_cgi(std::string file, std::map<std::string,std::string> &cgis , Request &request, std::string upload_path)
{
	std::string ext = file.substr(file.find_last_of('.') + 1);
    int status;
    std::string path = cgis[ext];
    if (path.empty())
        throw std::runtime_error("500");

    int input[2];
    int output[2];
    if (pipe(input) == -1 || pipe(output) == -1)
        throw std::runtime_error("500");
    char **argc = new char*[3];
    char **envp = get_env(setup_env(file, request, upload_path));
    pid_t pid = fork();
    if (pid == -1){
        delete[] argc;
        for (int i = 0; envp[i]; i++)
            delete[] envp[i];
        delete[] envp;
        throw std::runtime_error("500");
    }
    if (pid == 0)
    {
        if (chdir(file.substr(0, file.find_last_of('/')).c_str()) == -1)
            exit(127);
        argc[0] = (char *)path.c_str();
        std::string tmp = "./" + file.substr(file.find_last_of('/') + 1);
        argc[1] = (char *)tmp.c_str();
        argc[2] = NULL;
        if (dup2(input[0], 0) == -1 || dup2(output[1], 1) == -1 || dup2(output[1], 2) == -1)
            exit(127);
        close_pipe(input, output);
        if (execve(path.c_str(), argc, envp) == -1)
            exit(127);
    }
    else
    {
        int sendbuufer = request.get_raw_body().size();
        int start = time(NULL);
        int total = 0;
        while (total < sendbuufer)
        {
            int sent = write(input[1], request.get_raw_body().c_str() + total, sendbuufer - total);
            if (sent == -1)
            {
                close_pipe(input, output);
                throw std::runtime_error("500");
            }
            total += sent;
        }
        delete[] argc;
        for (int i = 0; envp[i]; i++)
            delete[] envp[i];
        delete[] envp;
        while (true)
        {
            if (time(NULL) - start > 5)
            {
                close_pipe(input, output);
                kill(pid, SIGKILL);
            }
            if (waitpid(pid, &status, WNOHANG) != 0)
                break;
        }
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
            throw std::runtime_error("500");
        else if (WIFSIGNALED(status)){
            throw std::runtime_error("504");
        }
        char buf[1024];
        std::string response;
        int len = 0;
        close(input[1]);
        close(output[1]);
        close(input[0]);
        bool found_content_type = false;
        int content_length = 0;
        int total_length = 0;
        size_t pos = 0;
        while ((len = read(output[0], buf, 1024)) > 0){
            total_length += len;
            response.append(buf,len);
            if (!found_content_type && (pos = response.find("\r\n\r\n")) != std::string::npos)
            {
                size_t pos = response.find("\r\n\r\n");
                std::string header = response.substr(0, pos + 2);
                std::map<std::string, std::string> headers;
                parse_headers(header, headers);
                if (headers.find("Content-Length") != headers.end()){
                    content_length = atoi(headers["Content-Length"].c_str());
                    found_content_type = true;
                }
            }
            if (total_length - pos >= (unsigned long)content_length && found_content_type){
                break;
            }
        }
        close(output[0]);
        setup_response(response, this->response);
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
