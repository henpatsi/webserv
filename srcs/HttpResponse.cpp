/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/15 11:02:12 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/19 07:42:54 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

// CONSTRUCTOR

HttpResponse::HttpResponse(HttpRequest& request)
{
	if (request.getFailResponseCode() != 0)
		setErrorValues(request.getFailResponseCode());
	else if (request.getMethod() == "GET")
		prepareGetResponse(request);
	else if (request.getMethod() == "POST")
		preparePostResponse(request);
	else if (request.getMethod() == "DELETE")
		prepareDeleteResponse(request);
	else
		setErrorValues(405);
	
	buildResponse();
}

// MEMBER FUNCTIONS

std::string getDefaultErrorMessage(int code)
{
	switch (code)
	{
		case 400:
			return "Bad Request";
		case 404:
			return "Not Found";
		case 405:
			return "Method Not Allowed";
		case 415:
			return "Unsupported Media Type";
		case 500:
			return "Internal Server Error";
		case 501:
			return "Not Implemented";
		default:
			return "Unknown Error";
	}
}

void HttpResponse::setErrorValues(int code, std::string message)
{
	this->responseCode = code;
	this->contentType = "text/html";

	if (message == "")
		message = getDefaultErrorMessage(code);

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
		setErrorValues(415);
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
		setErrorValues(404);
		return ;
	}

	// Read file content into content
	while (file.good())
	{
		std::string line;
		std::getline(file, line);
		if ((file.rdstate() & std::ios_base::badbit) != 0)
		{
			setErrorValues(500);
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
		setErrorValues(501);
	}
	else
		prepareGetResponse(request);
}

void HttpResponse::prepareDeleteResponse(HttpRequest& request)
{
	(void)request;
	setErrorValues(501);
}
