/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/15 16:46:08 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/17 14:18:23 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <unistd.h>

int main (int argc, char *argv[])
{
	if (argc != 4)
	{
		std::cout << "Invalid amount of arguments\nUse: ./test_client [ip] [port] [message]\n";
		return (1);
	}
	
	std::string ip = argv[1];
	std::string port = argv[2];
	std::string message = argv[3];
	std::cout << "Trying to connect to " << ip << ":" << port << "\n";

	// Get address info
	int status = 0;
	struct addrinfo hints;
	struct addrinfo *servinfo;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(ip.c_str(), port.c_str(), &hints, &servinfo) != 0)
	{
		std::cout << "getaddrinfo error: " << gai_strerror(status) << "\n";
		exit (1);
	}

	// Create socket
	int serverSocket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (serverSocket == -1)
	{
		perror("socket error");
		exit (1);
	}

	// Connect to server
	if (connect(serverSocket, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
	{
		perror("connect error");
		exit (1);
	}

	std::cout << "Successfully connected\n";

	freeaddrinfo(servinfo);

	// Send message
	int bytesSent = send(serverSocket, message.c_str(), strlen(message.c_str()), 0);
	if (bytesSent== -1)
	{
		perror("send error");
		exit (1);
	}

	std::cout << "Message sent\n";

	char buf[1024] = {0};
	int bytesReceived = recv(serverSocket, buf, 1024 - 1, 0);
	if (bytesReceived == -1)
	{
		perror("recv error");
		exit (1);
	}

	std::cout << "\nReceived:\n" << buf << "\n";

	close(serverSocket);

	return (0);
}
