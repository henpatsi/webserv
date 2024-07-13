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

		std::string getResponse(void);

	private:
		std::string response;
};

#endif
