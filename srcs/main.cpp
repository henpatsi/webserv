/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/10 11:37:37 by hpatsi            #+#    #+#             */
/*   Updated: 2024/08/05 08:52:10 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #include <iostream>
// #include <fstream>
// #include <exception>
// #include <ctime>

// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h> // inet_addr allowed?
// #include <unistd.h>
// #include <poll.h>
// #include <fcntl.h>

#include "ServerManager.hpp"
// #include "HttpRequest.hpp"
// #include "HttpResponse.hpp"
// #include "ServerConfig.hpp"

// int	exit_error(std::string message)
// {
// 	std::cerr << message << "\n";
// 	perror("");
// 	exit (1);
// }

// // VERY SIMPLISTIC IMPLEMENTATION FOR TESTING PURPOSES
// std::string buildPath(std::string requestPath, Route& route)
// {
// 	std::string path = route.root + requestPath;

// 	if (path.find(".") == std::string::npos && path.back() != '/')
// 		path += "/";

// 	if (path.back() == '/')
// 	{
// 		if (!route.directoryListing)
// 			path += "index.html";
// 		return path;
// 	}

// 	return path;
// }

// int create_server()
// {

// 	uint16_t port = 8080;
// 	std::string ip_address = "127.0.0.1";

// 	sockaddr_in serverAddress;

// 	// Create server socket
// 	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
// 	if (serverSocket == -1)
// 		exit_error("Failed to open server socket");

// 	// Set socket to non-blocking
// 	if (fcntl(serverSocket, F_SETFL, O_NONBLOCK) == -1)
// 		exit_error("Failed to set server socket to non-blocking");

// 	// Prevents "Address already in use" error message
// 	int yes=1;
// 	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
// 		exit_error("setsockopt failed");

// 	// Bind socket to address
// 	serverAddress.sin_family = AF_INET;
// 	serverAddress.sin_port = htons(port);
// 	serverAddress.sin_addr.s_addr = inet_addr(ip_address.c_str());
// 	if (bind(serverSocket, (sockaddr*) &serverAddress, sizeof(serverAddress)) == -1)
// 		exit_error("bind failed");

// 	// Listen for connections
// 	if (listen(serverSocket, 5) == -1)
// 		exit_error("listen failed");

// 	std::cout << "Server listening at addr " << inet_ntoa(serverAddress.sin_addr) << " port " << ntohs(serverAddress.sin_port) << "\n";

// 	return serverSocket;
// }

// int accept_connection(int serverSocket)
// {
// 	sockaddr_in connectionAddress;
// 	socklen_t connectionAddressLen = sizeof(connectionAddress);
// 	int connectionSocket = accept(serverSocket, (sockaddr*) &connectionAddress, &connectionAddressLen);
// 	if (connectionSocket == -1)
// 		exit_error("accept failed");
// 	if (fcntl(connectionSocket, F_SETFL, O_NONBLOCK) == -1) // Thought this is not necessary but was blocking in HttpRequest
// 		exit_error("Failed to set connection socket to non-blocking");

// 	time_t timeNow = std::time(nullptr);
// 	std::cout << "\n" << std::asctime(std::localtime(&timeNow)) << "Connected to " << inet_ntoa(connectionAddress.sin_addr) << " on port " << ntohs(connectionAddress.sin_port) << ", socket " << connectionSocket << "\n";

// 	return connectionSocket;
// }

// void handle_request(int connectionSocket)
// {
// 	// Parse request into HttpRequest object
// 	HttpRequest request = HttpRequest(connectionSocket);

// 	std::cout << "--- Request header received ---\n";

// 	// TEST FOR TRYING TO READ CONTENT UNTIL COMPLETE
// 	int failCount = 0;
// 	while (!request.isComplete())
// 	{
// 		request.tryReadContent(connectionSocket);
// 		failCount++;
// 		if (failCount > 1000)
// 		{
// 			request.setFailResponseCode(408);
// 			break;
// 		}
// 		// std::cout << "Read = " << request.getReadContentLength() << " Remaining = " << request.getRemainingContentLength() << "\n";
// 		std::this_thread::sleep_for(std::chrono::milliseconds(1));
// 	}
	
// 	std::cout << "--- Request content received ---\n\n";

// 	// HARD CODED ROUTE FOR TESTING PURPOSES
// 	Route route = Route();
// 	route.allowedMethods = ServerConfig::parseRequestMethod("GET") | ServerConfig::parseRequestMethod("POST") | ServerConfig::parseRequestMethod("DELETE");
// 	route.redirect = "";
// 	route.root = "www";
// 	route.directoryListing = true;
// 	route.defaultAnswer = ""; // What is this?
// 	route.acceptUpload = false;
// 	route.CGI = "";
// 	route.uploadDir = "www/uploads/";
// 	route.location = buildPath(request.getResourcePath(), route);

// 	std::cout << "Route location: " << route.location << "\n";

// 	// Build response into HttpResponse object
// 	HttpResponse response = HttpResponse(request, route);
// 	// std::cout << response.getResponse() << "\n";

// 	// Respond to client
// 	if (write(connectionSocket, response.getResponse().c_str(), response.getResponse().size()) == -1)
// 		exit_error("Writing response failed");
// }

int main(int argc, char *argv[])
{
	(void) argc;

	try
	{
		ServerManager s(argv[1]);
		s.runServers();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}


	// (void) argc;
	// (void) argv;

	// int serverSocket = create_server();

	// // Set up polling
	// pollfd pollfds[1];
	// pollfds[0].fd = serverSocket;
	// pollfds[0].events = POLLIN;

	// while (1)
	// {
	// 	int pollResult = poll(pollfds, 1, 1000);

	// 	if (pollResult == -1)
	// 		exit_error("poll failed");
	// 	else if (pollResult == 0) // No activity on server socket
	// 		continue ;
	// 	else if (pollfds[0].revents & POLLIN) // Connection detected
	// 	{
	// 		// Open connection to server on connection socket
	// 		int connectionSocket = accept_connection(serverSocket);
	// 		// Parse request, build response, and send response
	// 		handle_request(connectionSocket);
	// 		close(connectionSocket);
	// 	}
	// }
	// close(serverSocket);
}
