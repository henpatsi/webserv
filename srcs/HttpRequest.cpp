/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 16:29:53 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/18 17:39:29 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

// CONSTRUCTOR

std::string readRequestHeader(int socketFD) // Reads 1 byte at a time until it reaches end of header
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
				break ;
			sleep(1);
			continue ;
		}
		else if (bytesRead == 0)
			break ;
		failedReads = 0; // Reset timeout
		totalBytesRead += bytesRead;
		clientMessageBuffer[bytesRead] = '\0';
		requestString += clientMessageBuffer;
		// std::cout << "Read " << bytesRead << " bytes\n";
		// std::cout << "Request String:\n" << requestString << "\n";
		// std::cout << "End: " << (requestString.find("\r\n\r") != std::string::npos) << "\n";
		if (requestString.find("\r\n\r") != std::string::npos)
			break ;
	}

	std::cout << "\nRequest Message:\n" << requestString << "\n";
	return requestString;
}

void extractUrlParameters(std::map<std::string, std::string>& parametersMap, std::string parametersString)
{
	// TODO check how NGINX handles parameters that are formatted incorrectly
	// TODO handle multiple (repeated) ? in the url
	while (1) // TODO make into e.g. do while loop
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

HttpRequest::HttpRequest(int socketFD)
{
	std::string requestMessageString = readRequestHeader(socketFD);
	std::istringstream sstream(requestMessageString);

	// Assumes the first word in the request is the method
	sstream >> this->method;
	if (this->method.empty())
	{
		badRequest = true;
		return ;
	}

	// Assumes the second word in the request is the url
	std::string url;
	sstream >> url;
	// Extracts path from the url
	this->resourcePath = url.substr(0, url.find('?'));
	if (this->resourcePath.empty())
	{
		badRequest = true;
		return ;
	}
	// Extracts the parameters from the url into a map
	if (url.find('?') != std::string::npos && url.back() != '?')
		extractUrlParameters(this->urlParameters, url.substr(url.find('?') + 1));

	// Assumes the third word in the request is the http version
	sstream >> this->httpVersion;
	if (this->httpVersion.empty())
	{
		badRequest = true;
		return ;
	}

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
			badRequest = true;
			return ;
		}
		std::string key = line.substr(0, line.find(':'));
		std::string value = line.substr(line.find(':') + 2);
		value.erase(value.length() - 1);
		if (key.empty() || value.empty()) // TODO also check that not just spaces if necessary
		{
			//std::cout << "Invalid header: " << line << "\n";
			badRequest = true;
			return ;
		}
		this->headers[key] = value;
	}

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
					this->badRequest = true;
					break ;
				}
				sleep(1);
				continue ;
			}
			failedReads = 0; // Reset timeout
			totalBytesRead += bytesRead;
			contentBuffer[bytesRead] = '\0';
			this->content += contentBuffer;
		}
	}

	/* DEBUG PRINT */
	std::cout << "\nMethod: " << this->method << "\n";
	std::cout << "Resource path: " << this->resourcePath << "\n";
	std::cout << "HTTP version: " << this->httpVersion << "\n";
	std::cout << "Url Parameters:\n";
	for (std::map<std::string, std::string>::iterator it = this->urlParameters.begin(); it != this->urlParameters.end(); it++)
		std::cout << "  " << it->first << " = " << it->second << "\n";
	std::cout << "Headers:\n";
	for (std::map<std::string, std::string>::iterator it = this->headers.begin(); it != this->headers.end(); it++)
		std::cout << "  " << it->first << " = " << it->second << "\n";
	std::cout << "Content:\n " << this->content << "\n";
	std::cout << "REQUEST INFO FINISHED\n\n";
}
