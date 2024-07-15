/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 16:29:53 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/15 16:03:13 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

// CONSTRUCTOR

void extractParameters(std::map<std::string, std::string>& parametersMap, std::string parametersString)
{
	while (1) // TODO make into e.g. do while loop
	{
		std::string parameter = parametersString.substr(0, parametersString.find('&'));
		std::string key = parameter.substr(0, parameter.find('='));
		std::string value = parameter.substr(parameter.find('=') + 1, std::string::npos);
		parametersMap[key] = value;
		if (parametersString.find('&') == std::string::npos)
			break ;
		parametersString = parametersString.substr(parametersString.find('&') + 1, std::string::npos);
	}
}

HttpRequest::HttpRequest(std::string requestMessageString)
{
	std::istringstream sstream(requestMessageString);

	// Assumes the first word in the request is the method
	sstream >> this->method;

	// Assumes the second word in the request is the url
	std::string url;
	sstream >> url;
	// Extracts path from the url
	this->resourcePath = url.substr(0, url.find('?'));
	// Extracts the parameters from the url into a map
	if (url.find('?') != std::string::npos)
		extractParameters(this->urlParameters, url.substr(url.find('?') + 1, std::string::npos));

	// Reads the headers into a map
	std::string line;
	std::getline(sstream, line); // Skips the HTTP version
	while (std::getline(sstream, line))
	{
		if (line == "\r")
			break ;
		std::string key = line.substr(0, line.find(':'));
		std::string value = line.substr(line.find(':') + 2, std::string::npos);
		this->headers[key] = value;
	}

	// Reads the body content if content length specified
	if (this->headers.find("Content-Length") != this->headers.end())
	{
		int contentLength = std::stoi(this->headers["Content-Length"]);
		char contentBuffer[300000] = {}; // TODO make dynamic
		sstream.read(contentBuffer, contentLength);
		this->content = contentBuffer;
	}

	std::cout << "\nMethod: " << this->method << "\n";
	std::cout << "Resource path: " << this->resourcePath << "\n";
	std::cout << "Url Parameters:\n";
	for (std::map<std::string, std::string>::iterator it = this->urlParameters.begin(); it != this->urlParameters.end(); it++)
		std::cout << it->first << " = " << it->second << "\n";
	std::cout << "Headers:\n";
	for (std::map<std::string, std::string>::iterator it = this->headers.begin(); it != this->headers.end(); it++)
		std::cout << it->first << " = " << it->second << "\n";
	std::cout << "Content:\n " << this->content << "\n";
	std::cout << "REQUEST INFO FINISHED\n\n";
}
