#include <iostream>
#include <arpa/inet.h> 
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <iterator>

#include "Server.hpp"
#include "ServerManager.hpp"

Server::Server(ServerConfig _config) : config(_config)
{
    std::vector<sockaddr_in> serverAddress = config.getAddress();
    std::cout << "Server listening at addr " << inet_ntoa(serverAddress[0].sin_addr) << " on port(s):\n";
    serverSocketFDS = std::list<std::pair<int, bool>>();
    for (auto& address : serverAddress)
    {
        serverSocketFDS.push_back(std::pair<int, bool>(socket(AF_INET, SOCK_STREAM, 0), false));
        if (serverSocketFDS.back().first == -1)
            throw SocketOpenException();
        int yes = 1;
        if (setsockopt(serverSocketFDS.back().first, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
            throw SocketOptionException();
        if (bind(serverSocketFDS.back().first, (sockaddr*) &address, sizeof(serverAddress)) == -1)
            throw BindException();
        if (listen(serverSocketFDS.back().first, 5) == -1)
            throw ListenException();
        std::cout << "\t" << ntohs(address.sin_port) << "\n";
    }
}

Route Server::findCorrectRoute(HttpRequest request)
{
    Route fitting;
    bool isFound = false;
    std::string requestPath = request.getResourcePath();
    // just checks if resourcePath has the start of the location of the route
    for (Route route : config.getRoutes())
    {
        if (requestPath.compare(0, route.location.length(), route.location) != 0)
            continue;
        // set the current one as matching or replace if the current route matches "more"
        if (!isFound || fitting.location.length() < route.location.length())
            fitting = route;
        isFound = true;
    }
    if (isFound)
    {
        std::cout << "Route found!\n";
        std::cout << "root: " << fitting.root << "\n";
        std::cout << "location: " << fitting.location << "\n";
        return fitting;
    }
    else
        throw RouteException();
}

void Server::connect(int incommingFD, int socketFD) // Sets up the fd
{
    // Create connection, get header from request
    Connection connection;
    connection.fd = incommingFD;
    connection.connectTime = std::time(nullptr);
    connection.request = HttpRequest(incommingFD);

    for (std::list<std::pair<int, bool>>::iterator it = serverSocketFDS.begin(); it != serverSocketFDS.end(); ++it)
    {
        if (it->first == socketFD)
        {
            connection.socketFD = socketFD;
            it->second = true;
        }
    }

    listeningFDS.push_back(connection);
    

    std::cout << "Connected " << connection.fd << " to server\n";
}

void Server::disconnect(std::list<Connection>::iterator connectionIT)
{
    std::cout << "Disconnecting " << connectionIT->fd << " from server\n";

    listeningFDS.erase(connectionIT);

    for (std::list<std::pair<int, bool>>::iterator it = serverSocketFDS.begin(); it != serverSocketFDS.end(); ++it)
    {
        if (it->first == connectionIT->socketFD)
        {
            it->second = false;
        }
    }
}

void Server::getRequest(int fd)
{
    std::list<Connection>::iterator it;
    for (it = listeningFDS.begin(); it != listeningFDS.end(); ++it)
    {
        if (it->fd == fd)
            break;
    }

    if (!it->request.isComplete())
    {
        std::cout << "\n--- Reading request ---\n";
        it->request.readRequest();
    }
}

bool Server::respond(int fd)
{
    std::list<Connection>::iterator it;
    for (it = listeningFDS.begin(); it != listeningFDS.end(); ++it)
    {
        if (it->fd == fd)
            break;
    }

    if (it->request.isComplete())
    {
        if (it->request.getFailResponseCode() == 0) // Only find route if no previous error occured
        {
            std::cout << "\n--- Finding route ---\n";
            try
            {
                it->route = findCorrectRoute(it->request);
            }
            catch(const std::exception& e)
            {
                std::cout << "Server: FindCorrectRouteError: " << e.what() << "\n";
                it->request.setFailResponseCode(404);
            }
        }

        std::cout << "\n--- Responding to client ---\n";
        HttpResponse response(it->request, it->route);
        if (send(fd, &response.getResponse()[0], response.getResponse().size(), 0) == -1)
            throw ServerManager::ManagerRuntimeException("Failed to send response");
        disconnect(it);
        return (true);
    }

    return (false);
}

std::vector<int> Server::checkTimeouts()
{
    std::vector<int> timedOutFDs;

    std::time_t currentTime = std::time(nullptr);
    for (std::list<Connection>::iterator it = listeningFDS.begin(); it != listeningFDS.end(); ++it)
    {
        if (currentTime - it->connectTime > TIMEOUT_SEC)
        {
            std::cout << "Connection timed out on fd " << it->fd << "\n";
            timedOutFDs.push_back(it->fd);
            it->request.setFailResponseCode(408);
            respond(it->fd);
            it = listeningFDS.begin();
        }
    }

    return timedOutFDs;
}
