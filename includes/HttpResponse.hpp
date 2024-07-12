#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

# include "HttpRequest.hpp"

# include <string>
# include <iostream>
# include <fstream>

class HttpResponse
{
	public:
		HttpResponse(HttpRequest& request);

		std::string HttpResponse::getResponse(void);

	private:
		int code;
		std::string contentType;
		std::ifstream file;
		std::string content;
		std::string response;
};

#endif
