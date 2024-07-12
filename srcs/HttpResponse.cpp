#include "HttpResponse.hpp"

// CONSTRUCTOR

void buildErrorResponse(std::string& response, int code, std::string message)
{
	response = "HTTP/1.1 " + std::to_string(code) + "\r\n";
	response += "Content-Type: text/html\r\n\r\n";
	response += "<html><body><h1>" + std::to_string(code) + " ";
	response += message + "</h1></body></html>";
}

HttpResponse::HttpResponse(HttpRequest& request)
{
	if (request.getMethod() == "GET")
	{
		// TODO show directory instead of loading index.html by default?
		// Build path
		if (request.getResourcePath().back() == '/' || request.getResourcePath().find(".html") != std::string::npos)
		{
			this->filePath = "./html" + request.getResourcePath();
			if (this->filePath.back() == '/')
				this->filePath += "index.html";
		}
		else if (request.getResourcePath().find(".png") != std::string::npos || request.getResourcePath().find(".jpg") != std::string::npos)
		{
			this->filePath = "./images" + request.getResourcePath();
		}
		else
		{
			buildErrorResponse(response, 404, "Not Found");
			return ;
		}

		std::cout << "file path = " << this->filePath << "\n";

		// Try open file
		this->file.open(this->filePath);
		if (!this->file.good())
		{
			buildErrorResponse(response, 404, "Not Found");
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

		this->code = 200;

		this->response = "HTTP/1.1 " + std::to_string(this->code) + "\r\n";
		this->response += "Content-Type: " + this->contentType + "\r\n\r\n";
		this->response += this->content;

	}
	else
	{
		buildErrorResponse(response, 405, "Method Not Allowed");
		return ;
	}
}

// MEMBER FUNCTIONS

std::string HttpResponse::getResponse(void)
{
	return this->response;
}