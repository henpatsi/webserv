/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/15 11:02:12 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/31 18:58:22 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include <filesystem>
// CONSTRUCTOR

HttpResponse::HttpResponse(HttpRequest& request, Route& route) : route(route)
{
	try
	{
		if (request.getFailResponseCode() != 0)
			setErrorAndThrow(request.getFailResponseCode(), "Request failed");
		
		this->path = this->route.root + request.getResourcePath();
		if (this->path.find(".") == std::string::npos && this->path.back() != '/')
			this->path += "/"; // Add to end of dir if not yet
		std::cout << "Path: " << this->path << "\n";

		if (!(this->route.allowedMethods & ServerConfig::parseRequestMethod(request.getMethod())))
			setErrorAndThrow(405, "Method not allowed");

		if (request.getMethod() == "HEAD")
			prepareHeadResponse();
		else if (request.getMethod() == "GET")
			prepareGetResponse();
		else if (request.getMethod() == "POST")
			preparePostResponse(request);
		else if (request.getMethod() == "DELETE")
			prepareDeleteResponse();
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

	buildResponse(request);

	std::cout << "Response code: " << this->responseCode << "\n";
}

HttpResponse::HttpResponse(cgiResponse& response, Route& route) : route(route)
{
	try
	{
		if (response.getFailResponseCode() != 0)
			setErrorAndThrow(response.getFailResponseCode(), "Request failed");
		
		if (!(this->route.allowedMethods & ServerConfig::parseRequestMethod(response.getMethod())))
			setErrorAndThrow(405, "Method not allowed");

	}
	catch(ResponseException& e) // Known error, response ready to build
	{
		std::cerr << "ResponseException: " << e.what() << "\n";
	}
	catch(...) // Something that was not considered went wrong
	{
		setError(500);
	}

	buildResponse(response);

	std::cout << "Response code: " << this->responseCode << "\n";
}

// MEMBER FUNCTIONS

// Create error page with default message if none provided or other error
void HttpResponse::buildDefaultErrorContent(int code)
{
	std::string contentString;

	contentString = "<html><body><h1>" + std::to_string(code) + " ";
	contentString += this->defaultErrorMessages[code] + "</h1></body></html>";

	this->content.insert(this->content.end(), contentString.begin(), contentString.end());
}

void HttpResponse::setError(int code)
{
	this->responseCode = code;
	this->contentType = "text/html";

	if (this->customErrorPages[code] != "")
	{
		std::ifstream file;
		char contentBuffer[FILE_READ_SIZE] = {0};

		// Try open file
		file.open(this->route.root + this->customErrorPages[code]);
		if (!file.good())
		{
			buildDefaultErrorContent(code);
			return ;
		}

		// Read file content into content
		while (file.good())
		{
			file.read(contentBuffer, FILE_READ_SIZE);
			if ((file.rdstate() & std::ios_base::badbit) != 0)
			{
				buildDefaultErrorContent(code);
				return ;
			}
			this->content.insert(this->content.end(), contentBuffer, contentBuffer + FILE_READ_SIZE);
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

void HttpResponse::buildResponse(cgiResponse& response)
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
	this->response += response.getHeaders();
	this->response += "\r\n";

	if (request.getMethod() != "HEAD")
		this->response += this->content;
}

void HttpResponse::buildResponse(HttpRequest &request)
{
	std::string responseString;

	time_t timestamp = time(nullptr);
	struct tm *timedata = std::gmtime(&timestamp);
	char buffer[100] = {0};
	if (std::strftime(buffer, 99, "%a, %d %b %Y %T GMT", timedata) == 0)
		setError(500);

	responseString = "HTTP/1.1 " + std::to_string(this->responseCode) + "\r\n";
	responseString += "Date: " + std::string(buffer) + "\r\n";
	responseString += "Content-Type: " + this->contentType + "\r\n";
	responseString += "Content-Length: " + std::to_string(this->content.size()) + "\r\n";
	responseString += "\r\n";

	this->response.insert(this->response.end(), responseString.begin(), responseString.end());
	if (request.getMethod() != "HEAD")
		this->response.insert(this->response.end(), this->content.begin(), this->content.end());
}

void HttpResponse::buildDirectoryList(void)
{
	std::vector<std::string> files;
	std::string contentString;

	try
	{
		for (auto file : std::filesystem::directory_iterator(this->path))
			files.push_back(file.path().filename().string());
	}
	catch(const std::exception& e)
	{
		setErrorAndThrow(404, "Directory for listing not found or not accessible");
	}
	catch (...)
	{
		setErrorAndThrow(500, "Unknown error while listing directory");
	}

	this->contentType = "text/html";

	contentString = "<html><body>";
	contentString += "<h1>Directory listing for " + this->path + "</h1>";
	contentString += "<ul>";
	for (std::string file : files)
	{
		std::string noRootPath = this->path.substr(this->route.root.length());
		contentString += "<li><a href=\"" + noRootPath + file + "\">" + file + "</a></li>";
	}
	contentString += "</ul>";
	contentString += "</body></html>";

	this->content.insert(this->content.end(), contentString.begin(), contentString.end());

	this->responseCode = 200;
}

void HttpResponse::prepareHeadResponse(void)
{
	prepareGetResponse();

	// if (this->path == "/") // TODO remove, ubuntu_tester requires this for some reason
	// 	setErrorAndThrow(405, "HEAD not allowed for request path");
}

void HttpResponse::prepareGetResponse(void)
{
	if (this->directoryListingAllowed && (this->path.find(".") == std::string::npos))
	{
		buildDirectoryList();
		return ;
	}

	if (this->path.find(".html") != std::string::npos)
		this->contentType = "text/html";
	else if (this->path.find(".png") != std::string::npos)
		this->contentType = "image/png";
	else if (this->path.find(".jpg") != std::string::npos)
		this->contentType = "image/jpg";
	else if (this->path.find(".pdf") != std::string::npos)
		this->contentType = "application/pdf";

	std::ifstream file;
	char contentBuffer[FILE_READ_SIZE] = {0};

	// Try open file
	file.open(this->path);
	if (!file.good())
		setErrorAndThrow(404, "Failed to open file in GET");

	// Read file content into content
	while (file.good())
	{
		file.read(contentBuffer, FILE_READ_SIZE);
		if ((file.rdstate() & std::ios_base::badbit) != 0)
			setErrorAndThrow(500, "Failed to read file in GET");
		this->content.insert(this->content.end(), contentBuffer, contentBuffer + FILE_READ_SIZE);
	}

	this->responseCode = 200;
}

void HttpResponse::preparePostResponse(HttpRequest &request)
{
	if (this->route.uploadDir.back() != '/')
		this->route.uploadDir += "/";
	if (this->path == this->route.uploadDir)
	{
		if (access(this->path.c_str(), F_OK) == -1)
			setErrorAndThrow(404, "Upload directory not found");
		int ret = writeMultipartData(request.getMultipartData(), this->route.uploadDir);
		if (ret != 0)
			setErrorAndThrow(ret, "Failed to open / write multipart data to file");

		this->path = "www/html/success.html";
		prepareGetResponse();
		this->responseCode = 201;
	}
	else
		prepareGetResponse();
}

void HttpResponse::prepareDeleteResponse(void)
{
	if (this->path == this->route.uploadDir)
		setErrorAndThrow(405, "DELETE not allowed for request path");
	
	if (access(this->path.c_str(), F_OK) == -1)
	{
		this->path = "www/html/failure.html";
		prepareGetResponse();
		return ;
	}
	if (remove(this->path.c_str()) != 0)
		setErrorAndThrow(500, "Failed to delete file");
	
	this->path = "www/html/success.html";
	prepareGetResponse();
}

// HELPER FUNCTIONS

// Returns 0 for success, error code for failure
int writeMultipartData(std::vector<multipartData> dataVector, std::string directory)
{
	bool containsFiles = false;

	for (multipartData data : dataVector)
	{
		if (data.boundary != "") // Write nested multipart data
		{
			int ret = writeMultipartData(data.multipartDataVector, directory);
			if (ret == 0)
				return ret;
			containsFiles = true; // If no error, must have written a file
			continue ;
		}
		else if (data.filename == "")
			continue ;

		containsFiles = true;

		std::string path = directory + data.filename;
		std::ofstream file(path);

		std::cout << "path = " << path << "\n";

		if (!file.good()) // TODO Can one file fail and another succeed?
			return (500);

		file.write(data.data.data(), data.data.size()); // TODO check write success
		file.close();
	}

	if (!containsFiles)
		return (400);
	return (0);
}

// EXCEPTIONS

const char* HttpResponse::ResponseException::what() const throw()
{
	return this->message.c_str();
}
