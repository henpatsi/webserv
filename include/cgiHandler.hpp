#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP
#include "HttpRequest.hpp"
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include "ServerConfig.hpp"
#include <fcntl.h>

std::string	ntoa(sockaddr_in &address);

class	cgiHandler
{
	private:
	std::string	gateway;
	std::string	path_info;
	std::string	query_string;
	std::string	method;
	std::string	script_name;
	std::string	server_protocol;
	std::string	content_length;
	std::string	content_type;
	std::string	port;
	std::string	hostname;
	std::string	remote_address;
	public:
	std::pair <int, int>	runCGI(HttpRequest &request, ServerConfig &config, sockaddr_in &client_address, Route &route);	
	char	**create_envs(char **envs, HttpRequest &request, ServerConfig &config, sockaddr_in &client_address, Route& route);
	class RunCgiException : public std::exception
	{
		public:
			RunCgiException(std::string message) : message(message) {};
			virtual const char* what() const throw();
		private:
			std::string message;
	};
};

#endif
