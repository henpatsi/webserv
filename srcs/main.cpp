/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/10 11:37:37 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/16 11:29:53 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <fstream>
#include <exception>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // inet_addr allowed?
#include <unistd.h>
#include <poll.h>
#include <ServerManager.hpp>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

# ifndef REQUEST_READ_BUFFER_SIZE
#  define REQUEST_READ_BUFFER_SIZE 300000
# endif

int	return_error(std::string message)
{
	std::cerr << message << "\n";
	perror("");
	exit (1);
}

std::string readRequestMessage(int socketFD)
{
	std::string requestString;
	char clientMessageBuffer[REQUEST_READ_BUFFER_SIZE] = {0};
	
	// TODO check if full message read, read more if not
	// TODO make work with chunked enconding
	int readAmount = read(socketFD, clientMessageBuffer, sizeof(clientMessageBuffer) - 1);
	if (readAmount == -1)
		throw std::system_error();
	requestString += clientMessageBuffer;

	return requestString;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
		return_error("Invalid amount of arguments\nUse: ./webserv [configuration file]");
	// Get port and ip from config
	std::string configFilename = argv[1];

	ServerManager serverManager(configFilename);

	/*
	std::ifstream configFile;
	configFile.open(configFilename);
	if (!configFile.is_open())
		return_error("Failed to open file: " + configFilename);

	uint16_t port = 8080;
	std::string ip_address = "127.0.0.1";


	// Set server socket and address, 
	int serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocketFD == -1)
		return_error("Failed to open server socket");
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	serverAddress.sin_addr.s_addr = inet_addr(ip_address.c_str());

	// DEBUG: removes "Address already in use" error message
	int yes=1;
	if (setsockopt(serverSocketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
		return_error("setsockopt failed");
		return (1);
	}

	// Bind socket to address, turn on listen
	if (bind(serverSocketFD, (sockaddr*) &serverAddress, sizeof(serverAddress)) == -1)
		return_error("bind failed");
	if (listen(serverSocketFD, 5) == -1)
		return_error("listen failed");

	std::cout << "Server listening at addr " << inet_ntoa(serverAddress.sin_addr) << " port " << ntohs(serverAddress.sin_port) << "\n";

	while (1)
	{
		// Open connection to server on connection socket, set connection address
		sockaddr_in connectionAddress;
		socklen_t connectionAddressLen = sizeof(connectionAddress);
		int connectionSocketFD = accept(serverSocketFD, (sockaddr*) &connectionAddress, &connectionAddressLen);
		if (connectionSocketFD == -1)
			return_error("Failed to open connection socket");
		std::cout << "\n" << "Connected to " << inet_ntoa(connectionAddress.sin_addr) << " on port " << ntohs(connectionAddress.sin_port) << ", socket " << connectionSocketFD << "\n";

		// Read client message into string
		std::string requestMessageString = readRequestMessage(connectionSocketFD);
		std::cout << "\nRequest Message:\n" << requestMessageString << "\n";

		// Parse request into HttpRequest object
		HttpRequest request = HttpRequest(requestMessageString);

		// Build response into HttpResponse object
		HttpResponse response = HttpResponse(request);
		//std::cout << response.getResponse() << "\n";

		// Respond to client
		if (write(connectionSocketFD, response.getResponse().c_str(), response.getResponse().size()) == -1)
			return_error("Writing response failed");

		close(connectionSocketFD);
	}

	close(serverSocketFD);
	*/

}
