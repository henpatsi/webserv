#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP
#include "HttpRequest.hpp"
#include <string>

class	cgiHandler
{
	private:
		std::string	_cgiHeader;
		HttpRequest	&_request;
		std::stringstream	gateway;
		std::string	path_info;
		std::string	query_string;
		std::string	method;
		std::string	script_name;
		std::string	server_protocol;
		std::stringstream	content_length;
		std::string	content_type;
		std::string	CGIcommand;
	public:
		cgiHandler(HttpRequest &request);
		int	runCGI();	
		void	create_envs(char *envs[16]);
		std::string	prepare_query_string(std::string);
    
	class PipeException : std::exception {
        const char * what() const noexcept { return ("Pipe failed"); }
    };
	class CloseException : std::exception {
        const char * what() const noexcept { return ("Close failed"); }
    };
};

#endif
