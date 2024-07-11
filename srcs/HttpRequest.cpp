/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 16:29:53 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/11 16:47:02 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

// CONSTRUCTOR

HttpRequest::HttpRequest(int socketFD)
{
	char clientMessageBuffer[1024] = {};
	if (read(socketFD, clientMessageBuffer, sizeof(clientMessageBuffer)) == -1)
	{
		std::cout << "Reading client message failed";
		return ;
	}
	std::cout << clientMessageBuffer << "\n";
}

// DESTRUCTOR

HttpRequest::~HttpRequest(void)
{

}

// MEMBER FUNCTIONS


