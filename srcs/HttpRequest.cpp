/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 16:29:53 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/13 15:08:55 by hpatsi           ###   ########.fr       */
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

	// Gets the parameters from the url
	if (url.find('?') != std::string::npos)
		extractParameters(this->urlParameters, url.substr(url.find('?') + 1, std::string::npos));

	// Gets the parameters from the post request
	if (this->method == "POST")
	{
		// Get last line of request message
		std::string postParametersString;
		while (sstream.good())
			sstream >> postParametersString;
		extractParameters(this->postParameters, postParametersString);
	}

	std::cout << "Method: " << this->method << "\n";
	std::cout << "Resource path: " << this->resourcePath << "\n";
	std::cout << "Url Parameters:\n";
	for (std::map<std::string, std::string>::iterator it = this->urlParameters.begin(); it != this->urlParameters.end(); it++)
		std::cout << it->first << " = " << it->second << "\n";
	std::cout << "Post Parameters:\n";
	for (std::map<std::string, std::string>::iterator it = this->postParameters.begin(); it != this->postParameters.end(); it++)
		std::cout << it->first << " = " << it->second << "\n";
}
