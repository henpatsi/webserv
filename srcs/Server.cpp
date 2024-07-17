#include "Server.hpp"
#include <iostream>
#include <arpa/inet.h> 
#include <sys/socket.h>
#include <netinet/in.h>

Server::Server(ServerConfig _config) : config(_config)
{
    serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketFD == -1)
        throw SocketOpenException();
    int yes=1;
	if (setsockopt(serverSocketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
        throw SocketOptionException();
    sockaddr_in serverAddress = config.getAddress();
    if (bind(serverSocketFD, (sockaddr*) &serverAddress, sizeof(serverAddress)) == -1)
		throw BindException();
    if (listen(serverSocketFD, 5) == -1)
		throw ListenException();
    std::cout << "Server listening at addr " << inet_ntoa(serverAddress.sin_addr) << " port " << ntohs(serverAddress.sin_port) << "\n";
}

// constructs response based on the Routes contained in the server, depending on that routes settings
std::string Server::GetAnswer()
{
    return "stuff";
}