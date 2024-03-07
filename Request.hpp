#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

typedef struct Form
{
	std::string name;
	std::string value;
	bool 		is_file;
	std::string filename;
	std::string content_type;
} Form;

class Request
{
	private:
		std::string method;
		std::string path;
		std::string version;
		std::string b_boundary;
    	bool body;
		std::string requesr_status;
		std::map<std::string, std::string> headers;
		std::string body_type;
		std::vector<Form> form;
		std::string raw_body;
		std::string binary_body;
		std::string query;
	//private methods used for parsing
	private:
		void parse_request(std::string request);
		void set_method(std::string method);
		void set_path(std::string path);
		void set_version(std::string version);
		void set_host(std::string host);
		void parce_body(std::string body, std::string content_type);
		void creat_headers(std::string &str);
		void check_request();
		bool check_uri();
		void parse_form(std::string form, std::string boundary);
	public:
		Request(std::string request);
		std::string get_method() const;
		std::string get_path() const;
		std::string get_version() const;
		std::string get_host() const;
		std::map<std::string, std::string> get_headers() const;
		std::map<std::string, std::string> get_uncode_form() const;
		std::string get_raw_body() const;
		std::string get_binary_body() const;
		std::string get_status() const;
		std::string get_query() const;
		std::string get_boundry() const;
		std::vector<Form>  get_form() const;
		~Request();
};

std::string to_string(int num);
