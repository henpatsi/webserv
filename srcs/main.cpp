/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/10 11:37:37 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/11 16:01:59 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <fstream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // inet_addr allowed?
#include <unistd.h>
#include <poll.h>
#include <ServerManager.hpp>

int	return_error(std::string message)
{
	std::cerr << message << "\n";
	perror("");
	exit (1);
}

int main(int argc, char *argv[])
{
	if (argc != 2)
		return_error("Invalid amount of arguments\nUse: ./webserv [configuration file]");
	ServerManager manager(argv[1]);
	// // Get port and ip from config
	// std::string configFilename = argv[1];
	// std::ifstream configFile;
	// configFile.open(configFilename);
	// if (!configFile.is_open())
	// 	return_error("Failed to open file: " + configFilename);

	// uint16_t port = 8080;
	// std::string ip_address = "127.0.0.1";


	// // Set server socket and address, 
	// int serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	// if (serverSocketFD == -1)
	// 	return_error("Failed to open server socket");
	// sockaddr_in serverAddress;
	// serverAddress.sin_family = AF_INET;
	// serverAddress.sin_port = htons(port);
	// serverAddress.sin_addr.s_addr = inet_addr(ip_address.c_str());

	// // Setup poll
	// // pollfd pollFDs[2];
	// // pollFDs[0].fd = serverSocketFD;
	// // pollFDs[1].fd = serverSocketFD;
	// // pollFDs[0].events = POLLIN;
	// // pollFDs[1].events = POLLIN;

	// // Bind socket to address, turn on listen
	// if (bind(serverSocketFD, (sockaddr*) &serverAddress, sizeof(serverAddress)) == -1)
	// 	return_error("bind failed");
	// if (listen(serverSocketFD, 5) == -1)
	// 	return_error("listen failed");

	// std::cout << "Server listening at addr " << inet_ntoa(serverAddress.sin_addr) << " port " << ntohs(serverAddress.sin_port) << "\n";

	// while (1)
	// {
	// 	// Open connection to server on connection socket, set connection address
	// 	sockaddr_in connectionAddress;
	// 	socklen_t connectionAddressLen = sizeof(connectionAddress);
	// 	int connectionSocketFD = accept(serverSocketFD, (sockaddr*) &connectionAddress, &connectionAddressLen);
	// 	if (connectionSocketFD == -1)
	// 		return_error("Failed to open connection socket");
	// 	std::cout << "Connected to " << inet_ntoa(connectionAddress.sin_addr) << " on port " << ntohs(connectionAddress.sin_port) << ", socket " << connectionSocketFD << "\n";

	// 	// Read client message
	// 	char clientMessageBuffer[1024] = {};
	// 	if (read(connectionSocketFD, clientMessageBuffer, sizeof(clientMessageBuffer)) == -1)
	// 		return_error("Reading client message failed");
	// 	std::cout << clientMessageBuffer << "\n";

	// 	// Build response from file
	// 	std::string responseFilename = "./html/index.html";
	// 	std::ifstream responseFile;
	// 	responseFile.open(responseFilename);
	// 	if (!configFile.is_open())
	// 		return_error("Failed to open file: " + responseFilename);
	// 	std::string response = "HTTP/1.1 200\r\nContent-Type: text/html\r\n\r\n";
	// 	std::string line;
	// 	while (responseFile.good())
	// 	{
	// 		std::getline(responseFile, line);
	// 		if ((responseFile.rdstate() & std::ios_base::badbit) != 0)
	// 			return_error("Reading response failed");
	// 		response += line + "\n";
	// 	}

	// 	// Respond to client
	// 	if (write(connectionSocketFD, response.c_str(), response.size()) == -1)
	// 		return_error("Writing response failed");

	// 	close(connectionSocketFD);
	// }

	// close(serverSocketFD);


}
