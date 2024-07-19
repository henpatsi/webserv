/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 16:29:53 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/19 12:12:07 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

// CONSTRUCTOR

void debugPrint(HttpRequest request)
{
	/* DEBUG PRINT */
	std::cout << "\nMethod: " << request.getMethod() << "\n";
	std::cout << "Resource path: " << request.getResourcePath() << "\n";
	std::cout << "HTTP version: " << request.getHttpVersion() << "\n";
	std::cout << "Url parameters:\n";
	std::map<std::string, std::string> urlParameters = request.getUrlParameters();
	for (std::map<std::string, std::string>::iterator it = urlParameters.begin(); it != urlParameters.end(); it++)
		std::cout << "  " << it->first << " = " << it->second << "\n";
	std::cout << "Headers:\n";
	std::map<std::string, std::string> headers = request.getHeaders();
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++)
		std::cout << "  " << it->first << " = " << it->second << "\n";
	std::vector<multipartData> multipartDataVector = request.getMultipartData();
	if (multipartDataVector.size() > 0)
	{
		std::cout << "Multipart data:\n";
		for (multipartData data : multipartDataVector)
			std::cout << "Name: " << data.name << "\nFilename: " << data.filename << "\nContent-Type: " << data.contentType << "\nData: " << data.data << "\n";
	}
	else if (request.getUrlEncodedData().size() > 0)
	{
		std::cout << "Url encoded data:\n";
		std::map<std::string, std::string> urlEncodedData = request.getUrlEncodedData();
		for (std::map<std::string, std::string>::iterator it = urlEncodedData.begin(); it != urlEncodedData.end(); it++)
			std::cout << it->first << " = " << it->second << "\n";
	}
	else
		std::cout << "Raw content:\n " << request.getRawContent() << "\n";
	std::cout << "\nREQUEST INFO FINISHED\n\n";
}

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

void extractMultipartData(std::vector<multipartData>& multipartDataVector, std::string rawContent, std::string boundary)
{
	std::string boundaryStart = "--" + boundary;
	std::string boundaryEnd = "--" + boundary + "--";
	std::string boundaryContent = rawContent.substr(rawContent.find(boundaryStart) + boundaryStart.size());
	boundaryContent = boundaryContent.substr(0, boundaryContent.find(boundaryEnd));

	// For each section of the boundary, extract the data into multipartDataVector
	while (1)
	{
		multipartData data;
		std::string boundarySection = boundaryContent.substr(0, boundaryContent.find(boundaryStart));

		std::istringstream sstream(boundarySection);
		std::string line;

		getline(sstream, line); // Read the rest of the boundary line
		// Read relevant headers
		while (getline(sstream, line))
		{
			if (line.find("Content-Disposition:") != std::string::npos)
			{
				data.name = line.substr(line.find("name=\"") + 6);
				data.name.erase(data.name.find("\""));
				data.filename = line.substr(line.find("filename=\"") + 10);
				data.filename.erase(data.filename.find("\""));
			}
			else if (line.find("Content-Type:") != std::string::npos)
			{
				data.contentType = line.substr(line.find("Content-Type:") + 14);
				data.contentType.erase(data.contentType.length() - 1);
			}
			else if (line == "\r")
				break ;
		}

		// Read data
		while (getline(sstream, line))
			data.data += line + "\n";
		data.data.erase(data.data.size() - 1); // Remove the last newline

		multipartDataVector.push_back(data);

		if (boundaryContent.find(boundaryStart) == std::string::npos)
			break ;
		else
			boundaryContent = boundaryContent.substr(boundaryContent.find(boundaryStart) + boundaryStart.size());
	}
}

std::string HttpRequest::readRequestHeader(int socketFD) // Reads 1 byte at a time until it reaches end of header
{
	std::string requestString;
	char clientMessageBuffer[REQUEST_READ_BUFFER_SIZE] = {0};
	int bytesRead = 0;
	int totalBytesRead = 0;
	int failedReads = 0;
	
	while (totalBytesRead < MAX_HEADER_SIZE)
	{
		bytesRead = read(socketFD, clientMessageBuffer, REQUEST_READ_BUFFER_SIZE - 1);
		if (bytesRead == -1) // If nothing to read, wait 1 second and try again
		{
			failedReads += 1;
			if (failedReads > 2) // After 2 failed reads (2 sec timeout), break
			{
				this->failResponseCode = 408;
				break ;
			}
			sleep(1);
			continue ;
		}
		else if (bytesRead == 0)
			break ;
		failedReads = 0; // Reset timeout
		totalBytesRead += bytesRead;
		clientMessageBuffer[bytesRead] = '\0';
		requestString += clientMessageBuffer;
		if (requestString.find("\r\n\r\n") != std::string::npos)
			break ;
	}

	std::cout << "\nRequest Message:\n" << requestString << "\n";
	return requestString;
}

void HttpRequest::parseFirstLine(std::istringstream& sstream)
{
	// Assumes the first word in the request is the method
	sstream >> this->method;
	if (this->method.empty())
	{
		this->failResponseCode = 400;
		return ;
	}

	// Assumes the second word in the request is the url
	std::string url;
	sstream >> url;
	// Extracts path from the url
	this->resourcePath = url.substr(0, url.find('?'));
	if (this->resourcePath.empty())
	{
		this->failResponseCode = 400;
		return ;
	}
	// Extracts the parameters from the url into a map
	if (url.find('?') != std::string::npos && url.back() != '?')
		extractUrlParameters(this->urlParameters, url.substr(url.find('?') + 1));
	
	// Assumes the third word in the request is the http version
	sstream >> this->httpVersion;
	if (this->httpVersion.empty())
	{
		this->failResponseCode = 400;
		return ;
	}
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
		{
			//std::cout << "Invalid header: " << line << "\n";
			this->failResponseCode = 400;
			return ;
		}
		std::string key = line.substr(0, line.find(':'));
		std::string value = line.substr(line.find(':') + 2);
		value.erase(value.length() - 1);
		if (key.empty() || value.empty()) // TODO also check that not just spaces if necessary
		{
			//std::cout << "Invalid header: " << line << "\n";
			this->failResponseCode = 400;
			return ;
		}
		this->headers[key] = value;
	}
}

void HttpRequest::parseBody(int socketFD)
{
	// Reads the body content if content length specified
	if (this->headers.find("Content-Length") != this->headers.end())
	{
		int contentLength = std::stoi(this->headers["Content-Length"]);
		char contentBuffer[CONTENT_READ_BUFFER_SIZE] = {0}; // TODO make dynamic?
		int bytesRead = 0;
		int totalBytesRead = 0;
		int failedReads = 0;

		while (totalBytesRead < contentLength)
		{
			// might read something past contents if there is something to read
			// TODO make read amount be based on the content length
			bytesRead = read(socketFD, contentBuffer, CONTENT_READ_BUFFER_SIZE - 1);
			if (bytesRead == -1) // If nothing to read, wait 1 second and try again
			{
				failedReads += 1;
				if (failedReads > 2) // After 2 failed reads (2 sec timeout), break
				{
					this->failResponseCode = 408;
					break ;
				}
				sleep(1);
				continue ;
			}
			failedReads = 0; // Reset timeout
			totalBytesRead += bytesRead;
			contentBuffer[bytesRead] = '\0';
			this->rawContent += contentBuffer;
		}
	}

	// Extracts the data from the body content from known content types
	if (this->getHeader("Content-Type").find("multipart/form-data") != std::string::npos)
	{
		std::string boundary = this->getHeader("Content-Type");
		boundary = boundary.substr(boundary.find("boundary=") + 9);
		extractMultipartData(this->multipartDataVector, this->rawContent, boundary);
	}
	else if (this->getHeader("Content-Type").find("application/x-www-form-urlencoded") != std::string::npos)
	{
		extractUrlParameters(this->urlEncodedData, this->rawContent);
	}
}

HttpRequest::HttpRequest(int socketFD)
{
	std::string requestMessageString = readRequestHeader(socketFD);
	if (this->failResponseCode != 0)
		return ;
	std::istringstream sstream(requestMessageString);

	parseFirstLine(sstream);
	parseHeader(sstream);
	parseBody(socketFD);

	debugPrint(*this);
}
