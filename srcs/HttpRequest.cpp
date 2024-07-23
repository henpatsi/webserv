/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 16:29:53 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/23 10:53:23 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

// CONSTRUCTOR

HttpRequest::HttpRequest(int socketFD)
{
	try
	{
		std::string requestMessageString = readRequestHeader(socketFD);
		std::istringstream sstream(requestMessageString);

		parseFirstLine(sstream);
		parseHeader(sstream);
		parseBody(socketFD);

		debugPrint();
	}
	catch (RequestException& e)
	{
		std::cerr << e.what() << "\n";
	}
}

// MEMBER FUNCTIONS

void HttpRequest::setErrorAndThrow(int responseCode)
{
	this->failResponseCode = responseCode;
	throw RequestException();
}

void HttpRequest::debugPrint()
{
	/* DEBUG PRINT */
	std::cout << "\nMethod: " << this->method << "\n";
	std::cout << "Resource path: " << this->resourcePath << "\n";
	std::cout << "HTTP version: " << this->httpVersion << "\n";
	std::cout << "Url parameters:\n";
	for (auto param : this->urlParameters)
		std::cout << "  " << param.first << " = " << param.second << "\n";
	std::cout << "Headers:\n";
	for (auto param : this->headers)
		std::cout << "  " << param.first << " = " << param.second << "\n";

	//std::cout << "Raw content:\n  " << this->rawContent << "\n";
	if (this->multipartDataVector.size() > 0)
	{
		std::cout << "Multipart data:";
		for (multipartData data : this->multipartDataVector)
		{
			std::cout << "\n  Name: " << data.name << "\n  Filename: " << data.filename << "\n  Content-Type: " << data.contentType;
			// std::string dataString(data.data.begin(), data.data.end());
			// std::cout << "\n  Data: '" << dataString << "'\n";
		}
	}
	else if (this->urlEncodedData.size() > 0)
	{
		std::cout << "Url encoded data:\n";
		for (auto param : this->urlEncodedData)
			std::cout << "  " << param.first << " = " << param.second << "\n";
	}

	std::cout << "\nREQUEST INFO FINISHED\n\n";
}

std::string HttpRequest::readLine(int socketFD)
{
	// Reads 1 byte at a time until it reaches end of line to not read too much
	char readBuffer[2] = {0};
	int bytesRead = 0;
	int failedReads = 0;
	std::string line;

	while (1)
	{
		bytesRead = read(socketFD, readBuffer, 1);
		if (bytesRead == -1) // If nothing to read, wait 1 second and try again
		{
			failedReads += 1;
			if (failedReads > 2) // After 2 failed reads (2 sec timeout), throw exception
				setErrorAndThrow(408);
			sleep(1);
			continue ;
		}
		failedReads = 0; // Reset timeout
		if (bytesRead == 0)
			setErrorAndThrow(400);
		line += readBuffer[0];
		if (readBuffer[0] == '\n')
			break ;
	}
	return (line);
}

std::string HttpRequest::readRequestHeader(int socketFD)
{
	std::string requestString;
	
	while (requestString.size() < MAX_HEADER_SIZE)
	{
		std::string line = readLine(socketFD);
		if (line == "\r\n")
			break ;
		requestString += line;
	}

	std::cout << "\nRequest Message:\n" << requestString << "\n";
	return requestString;
}

void HttpRequest::readContent(int socketFD, int contentLength)
{
	char contentBuffer[CONTENT_READ_BUFFER_SIZE + 1] = {0};
	int contentReadSize;
	int bytesRead = 0;
	int failedReads = 0;

	while (contentLength > 0)
	{
		contentReadSize = contentLength > CONTENT_READ_BUFFER_SIZE ? CONTENT_READ_BUFFER_SIZE : contentLength;
		bytesRead = read(socketFD, contentBuffer, contentReadSize);
		if (bytesRead == -1) // If nothing to read, wait 1 second and try again
		{
			failedReads += 1;
			if (failedReads > 2) // After 2 failed reads (2 sec timeout), break
				setErrorAndThrow(408);
			sleep(1);
			continue ;
		}
		failedReads = 0; // Reset timeout
		contentBuffer[bytesRead] = '\0';
		this->rawContent.insert(this->rawContent.end(), contentBuffer, contentBuffer + bytesRead);
		contentLength -= bytesRead;
	}
}

void HttpRequest::readChunkedContent(int socketFD) // TODO chunked content missing newlines???
{
	std::string line = readLine(socketFD);
	int chunkSize = std::stoi(line.substr(0, line.find("\r")), 0, 16);
	while (chunkSize > 0)
	{
		readContent(socketFD, chunkSize);
		line = readLine(socketFD); // Reads the empty line
		line = readLine(socketFD);
		chunkSize = std::stoi(line.substr(0, line.find("\r")), 0, 16);
	}
}

void HttpRequest::parseFirstLine(std::istringstream& sstream)
{
	// Assumes the first word in the request is the method
	sstream >> this->method;
	if (this->method.empty())
		setErrorAndThrow(400);

	// Assumes the second word in the request is the url
	std::string url;
	sstream >> url;
	// Extracts path from the url
	this->resourcePath = url.substr(0, url.find('?'));
	if (this->resourcePath.empty())
		setErrorAndThrow(400);
	// Extracts the parameters from the url into a map
	if (url.find('?') != std::string::npos && url.back() != '?')
		extractUrlParameters(this->urlParameters, url.substr(url.find('?') + 1));
	
	// Assumes the third word in the request is the http version
	sstream >> this->httpVersion;
	if (this->httpVersion.empty())
		setErrorAndThrow(400);
}

void HttpRequest::parseHeader(std::istringstream& sstream)
{
	std::string line;
	std::getline(sstream, line); // Reads the rest of the first line
	// Reads the headers into a map
	while (std::getline(sstream, line))
	{
		if (line == "\r")
			break ;
		if (line.back() != '\r'
			|| line.find(':') == std::string::npos
			|| line[line.size() - 2] == ':')
			setErrorAndThrow(400);
		std::string key = line.substr(0, line.find(':'));
		std::string value = line.substr(line.find(':') + 2);
		value.erase(value.length() - 1);
		if (key.empty() || value.empty()) // TODO also check that not just spaces if necessary
			setErrorAndThrow(400);
		this->headers[key] = value;
	}
}

void HttpRequest::parseBody(int socketFD)
{
	// Reads the body content if content length specified
	if (this->headers["Transfer-Encoding"] == "chunked")
		readChunkedContent(socketFD);
	else if (this->headers.find("Content-Length") != this->headers.end())
		readContent(socketFD, std::stoi(this->headers["Content-Length"]));

	// Extracts the data from the body content from known content types
	if (this->getHeader("Content-Type").find("multipart/form-data") != std::string::npos)
	{
		std::string boundary = this->getHeader("Content-Type");
		boundary = boundary.substr(boundary.find("boundary=") + 9);
		if (extractMultipartData(this->multipartDataVector, this->rawContent, boundary) == -1)
			setErrorAndThrow(400);
	}
	else if (this->getHeader("Content-Type").find("application/x-www-form-urlencoded") != std::string::npos)
	{
		std::string rawContentString(this->rawContent.begin(), this->rawContent.end());
		extractUrlParameters(this->urlEncodedData, rawContentString);
	}
}

// HELPER FUNCTIONS

void extractUrlParameters(std::map<std::string, std::string>& parametersMap, std::string parametersString)
{
	// TODO check how NGINX handles parameters that are formatted incorrectly
	// TODO handle multiple (repeated) ? in the url
	while (1)
	{
		std::string parameter = parametersString.substr(0, parametersString.find('&'));
		if (parameter.find('=') != std::string::npos && parameter.front() != '=' && parameter.back() != '=')
		{
			std::string key = parameter.substr(0, parameter.find('='));
			std::string value = parameter.substr(parameter.find('=') + 1);
			parametersMap[key] = value;
		}
		if (parametersString.find('&') == std::string::npos || parametersString.find('&') == parametersString.size() - 1)
			break ;
		parametersString = parametersString.substr(parametersString.find('&') + 1);
	}
}

int extractMultipartData(std::vector<multipartData>& multipartDataVector, std::vector<char>& rawContent, std::string boundary)
{
	std::string boundaryStartString = "--" + boundary;
	std::string boundaryEndString = "--" + boundary + "--";
	std::string headerEndString = "\r\n\r\n";

	std::vector<char>::iterator start;
	std::vector<char>::iterator end;

	start = rawContent.begin();
	
	// Loop through each section of the multipart data
	while (!std::equal(boundaryEndString.begin(), boundaryEndString.end(), start))
	{
		multipartData data;

		// Check boundary start and skip over it
		if (!std::equal(boundaryStartString.begin(), boundaryStartString.end(), start))
			return (-1);
		start += boundaryStartString.size() + 2;

		// Get header end
		end = std::search(start, rawContent.end(), headerEndString.begin(), headerEndString.end());
		if (end == rawContent.end())
			return (-1);

		// Read relevant headers into multipartData
		std::string headerSection(start, end);
		std::istringstream sstream(headerSection);
		std::string line;
		while (getline(sstream, line))
		{
			if (line.find("Content-Disposition:") != std::string::npos)
			{
				if (line.find("name=\"") != std::string::npos)
				{
					data.name = line.substr(line.find("name=\"") + 6);
					data.name.erase(data.name.find("\""));
				}
				if (line.find("filename=\"") != std::string::npos)
				{
					data.filename = line.substr(line.find("filename=\"") + 10);
					data.filename.erase(data.filename.find("\""));
				}
			}
			else if (line.find("Content-Type:") != std::string::npos)
			{
				data.contentType = line.substr(line.find("Content-Type:") + 14);
				// Get boundary of nested multipart data
				if (data.contentType.find("boundary=") != std::string::npos)
				{
					data.boundary = data.contentType.substr(data.contentType.find("boundary=") + 9);
					data.contentType.erase(data.contentType.find(";"));
				}
			}
		}

		// Get body iterators
		start = end + headerEndString.size();
		end = std::search(start, rawContent.end(), boundaryStartString.begin(), boundaryStartString.end());
		if (end == rawContent.end())
			return (-1);

		// Copy body into multipartData
		data.data.insert(data.data.end(), start, end - 2);

		// Get nested multipart data
		if (!data.boundary.empty())
		{
			if (extractMultipartData(data.multipartDataVector, data.data, data.boundary) == -1)
				return (-1);
		}

		multipartDataVector.push_back(data);

		start = end;
	}
	return (1);
}
