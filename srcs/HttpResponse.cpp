/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/15 11:02:12 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/18 20:01:32 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

// CONSTRUCTOR

HttpResponse::HttpResponse(HttpRequest& request)
{
	if (request.isBadRequest())
		setErrorValues(400, "Bad Request");
	else if (request.getMethod() == "GET")
		prepareGetResponse(request);
	else if (request.getMethod() == "POST")
		preparePostResponse(request);
	else if (request.getMethod() == "DELETE")
		prepareDeleteResponse(request);
	else
		setErrorValues(405, "Method Not Allowed");
	
	buildResponse();
}

// MEMBER FUNCTIONS

void HttpResponse::setErrorValues(int code, std::string message)
{
	this->responseCode = code;
	this->contentType = "text/html";

	this->content = "<html><body><h1>" + std::to_string(code) + " ";
	this->content += message + "</h1></body></html>";
}

void HttpResponse::buildResponse()
{
	this->response = "HTTP/1.1 " + std::to_string(this->responseCode) + "\r\n";
	this->response += "Content-Type: " + this->contentType + "\r\n";
	this->response += "Content-Length: " + std::to_string(this->content.length()) + "\r\n";
	this->response += "\r\n";
	this->response += this->content;
}

void HttpResponse::buildPath(std::string requestPath)
{
	this->path = SITE_ROOT;

	// TODO do we even need any other types than html?
	if (requestPath.find(".png") != std::string::npos || requestPath.find(".jpg") != std::string::npos)
	{
		this->contentType = "image/" + requestPath.substr(requestPath.find(".") + 1);
		this->path += "images" + requestPath;
	}
	else if (requestPath.find(".pdf") != std::string::npos)
	{
		this->contentType = "application/pdf";
		this->path += "docs" + requestPath;
	}
	else if (requestPath.find(".html") != std::string::npos)
	{
		this->contentType = "text/html";
		this->path += "html" + requestPath;
	}
	else if (requestPath.find(".") == std::string::npos)
	{
		// TODO check if directory listing is allowed
		this->contentType = "text/html";
		this->path += "html" + requestPath;
		if (this->path.back() == '/')
			this->path += "index.html";
		else if (this->path.find(".html") == std::string::npos)
			this->path += "/index.html";
	}
	else
	{
		// TODO make this actually return
		setErrorValues(415, "Unsupported Media Type");
	}
}

void HttpResponse::prepareGetResponse(HttpRequest& request)
{
	std::ifstream file;

	buildPath(request.getResourcePath());

	// std::cout << "file path = " << filename << "\n";

	// Try open file
	file.open(this->path);
	if (!file.good())
	{
		setErrorValues(404, "Not Found");
		return ;
	}

	// Read file content into content
	while (file.good())
	{
		std::string line;
		std::getline(file, line);
		if ((file.rdstate() & std::ios_base::badbit) != 0)
		{
			setErrorValues(500, "Internal Server Error");
			return ;
		}
		this->content += line + "\n";
	}

	this->responseCode = 200;
}

void HttpResponse::preparePostResponse(HttpRequest& request)
{

	// TODO store the content in a not text format?

	if (request.getResourcePath() == "/upload") // Works for text file
	{
		std::string boundary = request.getHeader("Content-Type");
		boundary = boundary.substr(boundary.find("boundary=") + 9);

		std::string line;
		std::istringstream contentStream(request.getContent());

		// Get the filename line
		while (std::getline(contentStream, line))
		{
			if (line.find("filename=") != std::string::npos)
				break ;
		}

		std::string filename = line.substr(line.find("filename=") + 10);
		filename.erase(filename.size() - 2);
		//std::cout << "filename = " << filename << "\n";

		std::string path = SITE_ROOT;
		path += UPLOAD_DIR + filename;
		std::ofstream file(path);
		if (!file.good())
		{
			setErrorValues(500, "Internal Server Error");
			return ;
		}

		std::getline(contentStream, line); // Skip the line with content type
		std::getline(contentStream, line); // Skip the empty line
		// Read into the file until the end of the boundary
		bool fistLine = true;
		while (std::getline(contentStream, line))
		{
			if (line.find(boundary) != std::string::npos)
			{
				break ;
			}
			if (!fistLine)
				file << "\n";
			else
				fistLine = false;
			line.erase(line.size() - 1); // To prevent extra newline at the end of the file
			file << line;
		}
	}
	else
		prepareGetResponse(request);
}

void HttpResponse::prepareDeleteResponse(HttpRequest& request)
{
	(void)request;
	setErrorValues(501, "Not Implemented");
}
