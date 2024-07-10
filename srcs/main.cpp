/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/10 11:37:37 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/10 12:54:56 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <fstream>

#include <sys/socket.h>
#include <netinet/in.h>

int	return_error(std::string message)
{
	std::cerr << message << "\n";
	exit (1);
}

int main(int argc, char *argv[])
{
	if (argc != 2)
		return_error("Invalid amount of arguments\nUse: ./webserv [configuration file]");


	std::string configFilename = argv[1];
	//std::ifstream infile;
	//infile.open(configFilename);
	//if (!infile.is_open())
	//	return_error("Failed to open file: " + configFilename);


	int serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocketFD == -1)
		return_error("Failed to open server socket");
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(8080);
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	bind(serverSocketFD, (sockaddr*) &serverAddress, sizeof(serverAddress));
	listen(serverSocketFD, 5);

	while (1)
	{
		int clientSocketFD = accept(serverSocketFD, nullptr, nullptr);
		if (clientSocketFD == -1)
			return_error("Failed to open client socket");
		char buffer[1024] = {0};
		recv(clientSocketFD, buffer, sizeof(buffer), 0);
		std::cout << buffer << "\n";
	}
}
