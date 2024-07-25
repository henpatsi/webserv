/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/15 11:02:12 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/25 16:33:58 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

// CONSTRUCTOR

HttpResponse::HttpResponse(HttpRequest& request)
{
	try
	{
		if (request.getFailResponseCode() != 0)
			setErrorAndThrow(request.getFailResponseCode(), "Reuest failed");
		
		buildPath(request.getResourcePath());

		if (request.getMethod() == "GET")
			prepareGetResponse(request);
		else if (request.getMethod() == "POST")
			preparePostResponse(request);
		else if (request.getMethod() == "DELETE")
			prepareDeleteResponse(request);
		else
			setErrorAndThrow(501, "Request method not implemented");
	}
	catch(ResponseException& e) // Known error, response ready to build
	{
		std::cerr << "ResponseException: " << e.what() << "\n";
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
void HttpResponse::buildDefaultErrorContent(int code)
{
	this->content = "<html><body><h1>" + std::to_string(code) + " ";
	this->content += this->defaultErrorMessages[code] + "</h1></body></html>";
}

void HttpResponse::setError(int code)
{
	this->responseCode = code;
	this->contentType = "text/html";

	if (this->customErrorPages[code] != "")
	{
		std::ifstream file(SITE_ROOT + this->customErrorPages[code]);
		if (!file.good())
		{
			buildDefaultErrorContent(code);
			return ;
		}

		while (file.good())
		{
			std::string line;
			std::getline(file, line);
			if ((file.rdstate() & std::ios_base::badbit) != 0)
			{
				buildDefaultErrorContent(code);
				return ;
			}
			this->content += line + "\n";
		}
	}
	else
		buildDefaultErrorContent(code);
}

void HttpResponse::setErrorAndThrow(int code, std::string message)
{
	setError(code);
	throw ResponseException(message);
}

void HttpResponse::buildResponse()
{
	time_t timestamp = time(nullptr);
	struct tm *timedata = std::gmtime(&timestamp);
	char buffer[100] = {0};
	if (std::strftime(buffer, 99, "%a, %d %b %Y %T GMT", timedata) == 0)
		setError(500);

	this->response = "HTTP/1.1 " + std::to_string(this->responseCode) + "\r\n";
	this->response += "Date: " + std::string(buffer) + "\r\n";
	this->response += "Content-Type: " + this->contentType + "\r\n";
	this->response += "Content-Length: " + std::to_string(this->content.length()) + "\r\n";
	this->response += "\r\n";
	this->response += this->content;
}

void HttpResponse::buildDirectoryList(HttpRequest& request)
{
	std::vector<std::string> files;

	try
	{
		for (auto file : std::filesystem::directory_iterator(this->path))
			files.push_back(file.path().filename().string());
	}
	catch(const std::filesystem::filesystem_error& e)
	{
		setErrorAndThrow(404, "Directory for listing not found or not accessible");
	}
	catch (...)
	{
		setErrorAndThrow(500, "Unknown error while listing directory");
	}

	this->content = "<html><body>";
	this->content += "<h1>Directory listing for " + this->path + "</h1>";
	this->content += "<ul>";
	for (std::string file : files)
	{
		std::string path = request.getResourcePath();
		if (path.back() != '/')
			path += "/";
		this->content += "<li><a href=\"" + path + file + "\">" + file + "</a></li>";
	}
	this->content += "</ul>";
	this->content += "</body></html>";

	this->responseCode = 200;
}

// VERY SIMPLISTIC IMPLEMENTATION FOR TESTING PURPOSES
void HttpResponse::buildPath(std::string requestPath)
{
	this->path = SITE_ROOT;

	if (requestPath.substr(0, 8) == "/uploads")
		return ;

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
		setErrorAndThrow(415, "Unsupported media type in request path");
}

void HttpResponse::prepareGetResponse(HttpRequest& request)
{
	if (this->directoryListingAllowed && request.getResourcePath() != "/uploads" && (this->path.find(".") == std::string::npos))
	{
		buildDirectoryList(request);
		return ;
	}

	std::ifstream file;

	// Try open file
	file.open(this->path);
	if (!file.good())
		setErrorAndThrow(404, "Failed to open file in GET");

	// Read file content into content
	while (file.good())
	{
		std::string line;
		std::getline(file, line);
		if ((file.rdstate() & std::ios_base::badbit) != 0)
			setErrorAndThrow(500, "Failed to read file in GET");
		this->content += line + "\n";
	}

	this->responseCode = 200;
}

void HttpResponse::preparePostResponse(HttpRequest& request)
{
	// Example of POST where not allowed
	// TODO handle properly based on config
	if (request.getResourcePath() == "/")
		setErrorAndThrow(405, "POST not allowed for request path");

	if (request.getResourcePath() == "/uploads")
	{
		std::string directoryPath = SITE_ROOT;
		directoryPath += UPLOAD_DIR;
		if (access(directoryPath.c_str(), F_OK) == -1)
			setErrorAndThrow(404, "Upload directory not found");
		if (writeMultipartData(request.getMultipartData(), directoryPath) == -1)
			setErrorAndThrow(500, "Failed to open / write multipart data to file");

		buildPath("/success.html");
		prepareGetResponse(request);
	}
	else
		prepareGetResponse(request);
}

void HttpResponse::prepareDeleteResponse(HttpRequest& request)
{
	if (request.getResourcePath().substr(0, 8) != "/uploads")
		setErrorAndThrow(405, "DELETE not allowed for request path");
	
	std::string filePath = "www" + request.getResourcePath();
	if (access(filePath.c_str(), F_OK) == -1)
	{
		buildPath("/failure.html");
		prepareGetResponse(request);
		return ;
	}
	if (remove(filePath.c_str()) != 0)
		setErrorAndThrow(500, "Failed to delete file");
	
	buildPath("/success.html");
	prepareGetResponse(request);
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

		file.write(data.data.data(), data.data.size()); // TODO check write success
		file.close();
	}

	return (1);
}

// EXCEPTIONS

const char* HttpResponse::ResponseException::what() const throw()
{
	return this->message.c_str();
}