#include "HttpResponse.hpp"

// CONSTRUCTOR

void buildErrorResponse(std::string& response, int code, std::string message)
{
	response = "HTTP/1.1 " + std::to_string(code) + "\r\n";
	response += "Content-Type: text/html\r\n\r\n";
	response += "<html><body><h1>" + std::to_string(code);
	response += message + "</h1></body></html>";
}

HttpResponse::HttpResponse(HttpRequest& request)
{
	if (request.getMethod() == "GET")
	{

	}
	else
	{
		buildErrorResponse(response, 405, "Method Not Allowed");
		return ;
	}

	// Read file content into content
	while (this->file.good())
	{
		std::string line;
		std::getline(this->file, line);
		if ((this->file.rdstate() & std::ios_base::badbit) != 0)
		{
			buildErrorResponse(response, 500, "Internal Server Error");
			return ;
		}
		this->content += line + "\n";
	}

	this->response = "HTTP/1.1 " + std::to_string(this->code) + "\r\n";
	this->response += "Content-Type: " + this->contentType + "\r\n\r\n";
	this->response += this->content;
}

// MEMBER FUNCTIONS

std::string HttpResponse::getResponse(void)
{
	return this->response;
}