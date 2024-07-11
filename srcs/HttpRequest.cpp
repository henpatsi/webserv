/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 16:29:53 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/11 18:34:15 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

// CONSTRUCTOR

std::string readRequest(int socketFD)
{
	std::string requestString;
	char clientMessageBuffer[REQUEST_READ_BUFFER_SIZE] = {};
	
	// TODO check if full message read, read more if not
	if (read(socketFD, clientMessageBuffer, sizeof(clientMessageBuffer)) == -1)
		throw HttpRequest::ReadFailedException();

	requestString += clientMessageBuffer;
	return requestString;
}

HttpRequest::HttpRequest(int socketFD)
{
	std::string requestString = readRequest(socketFD);
	std::cout << requestString << "\n";

	std::istringstream sstream(requestString);
	std::string line;

	sstream >> this->method;
	std::cout << "Method: " << this->method << "\n";
	
	sstream >> line;
	this->resource_path = line.substr(0, line.find('?'));
	std::cout << "Resource path: " << this->resource_path << "\n";
}

// DESTRUCTOR

HttpRequest::~HttpRequest(void)
{

}

// MEMBER FUNCTIONS


