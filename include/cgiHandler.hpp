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
		std::string	_cgiHeader;
		std::stringstream	gateway;
		std::string	path_info;
		std::string	query_string;
		std::string	method;
		std::string	script_name;
		std::string	server_protocol;
		std::stringstream	content_length;
		std::string	content_type;
		std::string	port;
		std::string	hostname;
		std::string	remote_address;
	public:
	std::pair <int, int>	runCGI(HttpRequest &request, ServerConfig &config, sockaddr_in &client_address);	
		void	create_envs(char **envs, HttpRequest &request, ServerConfig &config, sockaddr_in &client_address);
    
	class PipeException : std::exception {
        const char * what() const noexcept { return ("Pipe failed"); }
    };
	class DupException : std::exception {
        const char * what() const noexcept { return ("Dup failed"); }
    };
	class CloseException : std::exception {
        const char * what() const noexcept { return ("Close failed"); }
    };
};

#endif
