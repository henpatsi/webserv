/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 16:29:53 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/12 08:53:18 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

// CONSTRUCTOR

HttpRequest::HttpRequest(std::string requestMessageString)
{
	std::istringstream sstream(requestMessageString);

	sstream >> this->method;
	std::cout << "Method: " << this->method << "\n";
	
	std::string url;
	sstream >> url;
	this->resourcePath = url.substr(0, url.find('?'));
	std::cout << "Resource path: " << this->resourcePath << "\n";

	if (url.find('?') != std::string::npos)
	{
		std::string parameters = url.substr(url.find('?') + 1, std::string::npos);

		while (1)
		{
			std::string parameter = parameters.substr(0, parameters.find('&'));
			std::string key = parameter.substr(0, parameter.find('='));
			std::string value = parameter.substr(parameter.find('=') + 1, std::string::npos);
			this->paramenters[key] = value;
			if (parameters.find('&') == std::string::npos)
				break ;
			parameters = parameters.substr(parameters.find('&') + 1, std::string::npos);
		}
	}

	std::cout << "Parameters:\n";
	for (std::map<std::string, std::string>::iterator it = this->paramenters.begin(); it != this->paramenters.end(); it++)
		std::cout << it->first << " = " << it->second << "\n";
}
