/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/15 11:02:12 by hpatsi            #+#    #+#             */
/*   Updated: 2024/08/22 14:30:34 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

// CONSTRUCTOR

HttpResponse::HttpResponse(HttpRequest& request, Route& route, std::string errorPage) : route(route), errorPage(errorPage)
{
	try
	{
		if (request.getFailResponseCode() == 307)
		{
			this->responseCode = request.getFailResponseCode();
			buildRedirectResponse();
			std::cerr << "Response code: " << this->responseCode << "\n";
			return ;
		}

		if (request.getFailResponseCode() != 0)
			setErrorAndThrow(request.getFailResponseCode(), "Request failed");
		
		this->path = this->route.root + request.getResourcePath();
		if (std::filesystem::is_directory(this->path))
		{
			if (this->path.back() != '/')
				this->path += "/"; // Standardize dir to end in /
		}
		std::cerr << "Path: " << this->path << "\n";

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
		std::cerr << "ResponseException: Unknown error\n";
		setError(500);
	}

	buildResponse(request);

	std::cerr << "Response code: " << this->responseCode << "\n";
}

HttpResponse::HttpResponse(cgiResponse& response, Route& route) : route(route)
{
	try
	{
		if (response.getFailResponseCode() == 0)
			this->responseCode = 200;
		else
			setError(response.getFailResponseCode());
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
}

// MEMBER FUNCTIONS

void HttpResponse::setError(int code)
{
	this->responseCode = code;

	if (this->errorPage != "")
		buildCustomErrorContent(code);
	else
		buildDefaultErrorContent(code);
}

void HttpResponse::setErrorAndThrow(int code, std::string message)
{
	setError(code);
	throw ResponseException(message);
}

void HttpResponse::buildDefaultSuccessContent(void)
{
	std::string contentString;

	contentString = "<!DOCTYPE html><html><body><h1>Success</h1></body></html>";

	this->content.insert(this->content.end(), contentString.begin(), contentString.end());
	this->contentType = "text/html";
}

void HttpResponse::buildDefaultErrorContent(int code)
{
	std::string contentString;

	contentString = "<!DOCTYPE html><html><body><h1>" + std::to_string(code) + " ";
	contentString += this->defaultErrorMessages[code] + "</h1></body></html>";

	this->content.insert(this->content.end(), contentString.begin(), contentString.end());
	this->contentType = "text/html";
}

void HttpResponse::buildCustomErrorContent(int code)
{
	if (readBinaryFile(this->errorPage, this->content) != 0)
	{
		std::cerr << "Custom error page could not be read\n";
		buildDefaultErrorContent(code);
		return ;
	}

	// Replace error code placeholder with code
	std::string errorCodePlaceholder = "<%ERROR%>";
	std::vector<char>::iterator it = std::search(this->content.begin(), this->content.end(), errorCodePlaceholder.begin(), errorCodePlaceholder.end());
	if (it != this->content.end())
	{
		try
		{
			std::string codeString = std::to_string(code);
			this->content.erase(it, safeNext(it, this->content.end(), errorCodePlaceholder.length()));
			this->content.insert(it, codeString.begin(), codeString.end());
		}
		catch(const std::exception& e)
		{
			std::cerr << "Error inserting code into custom page\n";
			buildDefaultErrorContent(code);
			return ;
		}
	}
	
	this->contentType = "text/html";
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
	this->contentType = "text/html";

	this->responseCode = 200;
}

void HttpResponse::buildResponse(cgiResponse& response)
{
	time_t timestamp = time(nullptr);
	struct tm *timedata = std::gmtime(&timestamp);
	char buffer[100] = {0};
	std::string responseString;
	if (std::strftime(buffer, 99, "%a, %d %b %Y %T GMT", timedata) == 0)
		setError(500);

	responseString = "HTTP/1.1 " + std::to_string(this->responseCode) + "\r\n";
	responseString += "Date: " + std::string(buffer) + "\r\n";
	responseString += response.getHeaders();
	responseString += "\r\n";

	this->response.insert(this->response.end(), responseString.begin(), responseString.end());
	if (response.getMethod() != "HEAD")
	{
		this->response.insert(this->response.end(), this->content.begin(), this->content.end());
		this->response.insert(this->response.end(), response.getContent().begin(), response.getContent().end());
	}
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

void HttpResponse::buildRedirectResponse(void)
{
	std::string responseString;

	responseString = "HTTP/1.1 307 Temporary Redirect\r\n";
	responseString += "Location: " + this->route.redirect + "\r\n";

	this->response.insert(this->response.end(), responseString.begin(), responseString.end());
}

void HttpResponse::prepareHeadResponse(void)
{
	prepareGetResponse();
}

void HttpResponse::prepareGetResponse(void)
{
	if (std::filesystem::is_directory(this->path))
	{
		if (this->route.directoryListing)
		{
			buildDirectoryList();
			return ;
		}
		
		if (this->route.defaultAnswer == "")
			setErrorAndThrow(403, "Directory listing not allowed");
		this->path += this->route.defaultAnswer;
	}

	if (this->path.find(".html") != std::string::npos)
		this->contentType = "text/html";
	else if (this->path.find(".png") != std::string::npos)
		this->contentType = "image/png";
	else if (this->path.find(".jpg") != std::string::npos)
		this->contentType = "image/jpg";
	else if (this->path.find(".pdf") != std::string::npos)
		this->contentType = "application/pdf";
	else
		this->contentType = "binary/octet-stream";

	int ret = readBinaryFile(this->path, this->content);
	if (ret != 0)
		setErrorAndThrow(ret, "Failed to read file in GET");

	this->responseCode = 200;
}

void HttpResponse::preparePostResponse(HttpRequest &request)
{
	if (this->route.uploadDir.back() != '/') // Standardize uploadDir path to end in /
		this->route.uploadDir += "/";

	if (multipartDataContainsFile(request.getMultipartData())) // Check if there are any files
	{
		if (this->route.acceptUpload == false)
			setErrorAndThrow(403, "Uploading not allowed for request path");
		if (this->route.uploadDir == "")
			setErrorAndThrow(403, "Upload directory not set for request path");
		if (access(this->route.uploadDir.c_str(), F_OK) == -1)
			setErrorAndThrow(403, "Upload directory not accessible");

		int ret = writeMultipartData(request.getMultipartData(), this->route.uploadDir);
		if (ret != 0)
			setErrorAndThrow(ret, "Failed to open / write multipart data to file");

		this->responseCode = 201;
		buildDefaultSuccessContent();
	}
	else
		prepareGetResponse();
}

void HttpResponse::prepareDeleteResponse(void)
{
	if (access(this->path.c_str(), F_OK) == -1)
	{
		if (errno == EACCES)
			setErrorAndThrow(403, "Access denied");
		else
			setErrorAndThrow(404, "File not found");
	}
	
	if (remove(this->path.c_str()) != 0)
		setErrorAndThrow(500, "Failed to delete file");

	this->responseCode = 201;
	buildDefaultSuccessContent();
}

// HELPER FUNCTIONS

int readBinaryFile(std::string path, std::vector<char>& dest)
{
	if (!std::filesystem::exists(path))
		return 404;
	if (access(path.c_str(), R_OK) == -1)
		return 403;

	std::ifstream file(path, std::ifstream::binary);
	if (!file.good())
		return 500;

	try
	{
		file.unsetf(std::ios::skipws); // Prevents skipping spaces
		// Set content size to filesize
		file.seekg(0, std::ios::end);
		dest.reserve(file.tellg());
		file.seekg(0, std::ios::beg);
		// Read file content into content
		dest.insert(dest.begin(), std::istream_iterator<char>(file), std::istream_iterator<char>());
		file.close();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return 500;
	}

	return 0;
}

// Returns 0 for success, error code for failure
int writeMultipartData(std::vector<multipartData> dataVector, std::string directory)
{
	for (multipartData data : dataVector)
	{
		if (data.boundary != "") // Write nested multipart data
		{
			int ret = writeMultipartData(data.multipartDataVector, directory);
			if (ret != 0)
				return ret;
			continue ;
		}
		else if (data.filename == "")
			continue ;

		std::string path = directory + data.filename;
		std::ofstream file(path);

		if (!file.good())
			return (500);
		if (!file.write(data.data.data(), data.data.size()))
			return (500);
		file.close();
	}

	return (0);
}

bool multipartDataContainsFile(std::vector<multipartData> dataVector)
{
	if (dataVector.empty())
		return false;

	for (multipartData data : dataVector)
	{
		if (data.filename != "")
			return true;
		if (data.boundary != "") // Check nested multipart data
		{
			if (multipartDataContainsFile(data.multipartDataVector))
				return true;
		}
	}
	return false;
}

// EXCEPTIONS

const char* HttpResponse::ResponseException::what() const throw()
{
	return this->message.c_str();
}
