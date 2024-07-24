/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/15 11:02:12 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/24 11:29:02 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

// CONSTRUCTOR

HttpResponse::HttpResponse(HttpRequest& request)
{
	try
	{
		if (request.getFailResponseCode() != 0)
			setErrorAndThrow(request.getFailResponseCode());
		
		buildPath(request.getResourcePath());

		if (request.getMethod() == "GET")
			prepareGetResponse(request);
		else if (request.getMethod() == "POST")
			preparePostResponse(request);
		else if (request.getMethod() == "DELETE")
			prepareDeleteResponse(request);
		else
			setErrorAndThrow(501);
	}
	catch(const ResponseException& e) // Known error, response ready to build
	{
		std::cerr << e.what() << '\n';
	}
	catch(...) // Something that was not considered went wrong
	{
		setError(500);
	}

	buildResponse();

	std::cout << "Response code: " << this->responseCode << "\n";
}

// MEMBER FUNCTIONS

// Create error page with default message if none provided or other error
void HttpResponse::setDefaultError(int code, std::string message)
{
	if (message == "")
		message = this->defaultErrorMessages[code];

	this->content = "<html><body><h1>" + std::to_string(code) + " ";
	this->content += message + "</h1></body></html>";
}

void HttpResponse::setError(int code, std::string message)
{
	this->responseCode = code;
	this->contentType = "text/html";

	if (this->customErrorPages[code] != "")
	{
		std::ifstream file(SITE_ROOT + this->customErrorPages[code]);
		if (!file.good())
		{
			setDefaultError(code);
			return ;
		}

		while (file.good())
		{
			std::string line;
			std::getline(file, line);
			if ((file.rdstate() & std::ios_base::badbit) != 0)
			{
				setDefaultError(code);
				return ;
			}
			this->content += line + "\n";
		}
	}
	else
		setDefaultError(code, message);
}

void HttpResponse::setErrorAndThrow(int code, std::string message)
{
	setError(code, message);
	throw ResponseException();
}

void HttpResponse::buildResponse()
{
	this->response = "HTTP/1.1 " + std::to_string(this->responseCode) + "\r\n";
	this->response += "Content-Type: " + this->contentType + "\r\n";
	this->response += "Content-Length: " + std::to_string(this->content.length()) + "\r\n";
	this->response += "\r\n";
	this->response += this->content;
}

void HttpResponse::buildDirectoryList() // TODO make sure links in subdirs work
{
	std::vector<std::string> files;

	try
	{
		for (auto file : std::filesystem::directory_iterator(this->path))
			files.push_back(file.path().filename().string());
	}
	catch(const std::filesystem::filesystem_error& e)
	{
		setErrorAndThrow(404);
	}
	catch (...)
	{
		setErrorAndThrow(500);
	}

	this->content = "<html><body>";
	this->content += "<h1>Directory listing for " + this->path + "</h1>";
	this->content += "<ul>";
	for (std::string file : files)
		this->content += "<li><a href=\"" + file + "\">" + file + "</a></li>";
	this->content += "</ul>";
	this->content += "</body></html>";

	this->responseCode = 200;
}

// VERY SIMPLISTIC IMPLEMENTATION FOR TESTING PURPOSES
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
		this->contentType = "text/html";
		this->path += "html" + requestPath;

		if (this->directoryListingAllowed)
			return;

		if (this->path.back() == '/')
			this->path += "index.html";
		else if (this->path.find(".html") == std::string::npos)
			this->path += "/index.html";
	}
	else
		setErrorAndThrow(415);
}

void HttpResponse::prepareGetResponse(HttpRequest& request)
{
	if (this->directoryListingAllowed && request.getResourcePath() != "/uploads" && (this->path.find(".") == std::string::npos))
	{
		buildDirectoryList();
		return ;
	}

	std::ifstream file;

	// Try open file
	file.open(this->path);
	if (!file.good())
		setErrorAndThrow(404);

	// Read file content into content
	while (file.good())
	{
		std::string line;
		std::getline(file, line);
		if ((file.rdstate() & std::ios_base::badbit) != 0)
			setErrorAndThrow(500);
		this->content += line + "\n";
	}

	this->responseCode = 200;
}

void HttpResponse::preparePostResponse(HttpRequest& request)
{
	// Example of POST where not allowed
	// TODO handle properly based on config
	if (request.getResourcePath() == "/")
		setErrorAndThrow(405);

	if (request.getResourcePath() == "/uploads")
	{
		std::string directoryPath = SITE_ROOT;
		directoryPath += UPLOAD_DIR;
		if (writeMultipartData(request.getMultipartData(), directoryPath) == -1)
			setErrorAndThrow(500);

		buildPath("/success.html");
		prepareGetResponse(request);
	}
	else
		prepareGetResponse(request);
}

void HttpResponse::prepareDeleteResponse(HttpRequest& request)
{
	(void)request;
	setErrorAndThrow(501);
}

// HELPER FUNCTIONS

int writeMultipartData(std::vector<multipartData> dataVector, std::string directory)
{
	for (multipartData data : dataVector)
	{
		if (data.boundary != "")
		{
			writeMultipartData(data.multipartDataVector, directory);
			continue ;
		}
		else if (data.filename == "")
			continue ;

		std::string path = directory + data.filename;
		std::ofstream file(path);

		if (!file.good())
			return (-1);

		file.write(data.data.data(), data.data.size());
		file.close();
	}

	return (1);
}
