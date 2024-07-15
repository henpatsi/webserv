#include "HttpResponse.hpp"

// CONSTRUCTOR

void buildResponse(std::string& response, int code, std::string contentType, std::string content)
{
	response = "HTTP/1.1 " + std::to_string(code) + "\r\n";
	response += "Content-Type: " + contentType + "\r\n\r\n";
	response += content;
}

void buildErrorResponse(std::string& response, int code, std::string message)
{
	std::string content;

	content = "<html><body><h1>" + std::to_string(code) + " ";
	content += message + "</h1></body></html>";

	buildResponse(response, code, "text/html", content);
}

void buildGetResponse(std::string& response, HttpRequest& request)
{
	std::string filename = SITE_ROOT;
	std::ifstream file;
	std::string content;
	std::string contentType;
	int responseCode;

	// TODO check if directory listing is allowed
	// Build path
	if (request.getResourcePath().back() == '/' || request.getResourcePath().find(".html") != std::string::npos)
	{
		contentType = "text/html";
		filename += "html" + request.getResourcePath();
		if (filename.back() == '/')
			filename += "index.html";
	}
	else if (request.getResourcePath().find(".png") != std::string::npos || request.getResourcePath().find(".jpg") != std::string::npos)
	{
		contentType = "image/" + request.getResourcePath().substr(request.getResourcePath().find(".") + 1);
		filename += "images" + request.getResourcePath();
	}
	else if (request.getResourcePath().find(".pdf") != std::string::npos)
	{
		contentType = "application/pdf";
		filename += "docs" + request.getResourcePath();
	}
	else
	{
		buildErrorResponse(response, 400, "Bad Request");
		return ;
	}

	// std::cout << "file path = " << filename << "\n";

	// Try open file
	file.open(filename);
	if (!file.good())
	{
		buildErrorResponse(response, 404, "Not Found");
		return ;
	}

	// Read file content into content
	while (file.good())
	{
		std::string line;
		std::getline(file, line);
		if ((file.rdstate() & std::ios_base::badbit) != 0)
		{
			buildErrorResponse(response, 500, "Internal Server Error");
			return ;
		}
		content += line + "\n";
	}

	responseCode = 200;

	buildResponse(response, responseCode, contentType, content);
}

HttpResponse::HttpResponse(HttpRequest& request)
{
	if (request.getMethod() == "GET")
		buildGetResponse(this->response, request);
	else if (request.getMethod() == "POST")
		buildErrorResponse(response, 42, "Method Not Yet Implemented");
	else if (request.getMethod() == "DELETE")
		buildErrorResponse(response, 42, "Method Not Yet Implemented");
	else
		buildErrorResponse(response, 405, "Method Not Allowed");
}

// MEMBER FUNCTIONS

std::string HttpResponse::getResponse(void)
{
	return this->response;
}
