#include "HttpResponse.hpp"

// CONSTRUCTOR

void buildErrorResponse(std::string& response, int code, std::string message)
{
	response = "HTTP/1.1 " + std::to_string(code) + "\r\n";
	response += "Content-Type: text/html\r\n\r\n";
	response += "<html><body><h1>" + std::to_string(code) + " ";
	response += message + "</h1></body></html>";
}

void buildGetResponse(std::string& response, HttpRequest& request)
{
	std::string filename;
	std::ifstream file;
	std::string content;
	std::string contentType;
	int responseCode;

	// TODO show directory instead of loading index.html by default?
	// Build path
	if (request.getResourcePath().back() == '/' || request.getResourcePath().find(".html") != std::string::npos)
	{
		contentType = "text/html";
		filename = "./html" + request.getResourcePath();
		if (filename.back() == '/')
			filename += "index.html";
	}
	else if (request.getResourcePath().find(".png") != std::string::npos || request.getResourcePath().find(".jpg") != std::string::npos)
	{
		contentType = "image/" + request.getResourcePath().substr(request.getResourcePath().find(".") + 1);
		filename = "./images" + request.getResourcePath();
	}
	else if (request.getResourcePath().find(".pdf") != std::string::npos)
	{
		contentType = "application/pdf";
		filename = "./docs" + request.getResourcePath();
	}
	else
	{
		buildErrorResponse(response, 404, "Not Found");
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

	response = "HTTP/1.1 " + std::to_string(responseCode) + "\r\n";
	response += "Content-Type: " + contentType + "\r\n\r\n";
	response += content;
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
