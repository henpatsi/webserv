#include <iostream>
#include <arpa/inet.h> 
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <iterator>
#include <random>
#include <limits>

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
    sessions = std::list<Session>();
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
        if (request.isCgi())
        {
	    std::string cgiPath = fitting.root;
            cgiPath += request.getResourcePath();
            std::cout << "cgi Path: " << cgiPath << std::endl;
            if (access(cgiPath.c_str(), F_OK) != 0)
            {
                std::cout << "CGI doesn't exist" << std::endl;
                throw RouteException();
            }
            if (access(cgiPath.c_str(), F_OK | X_OK) != 0)
            {
                std::cout << "CGI is not executable" << std::endl;
                throw RouteException();
            }
        }
        return fitting;
    }
    else
        throw RouteException();
}

void Server::connect(int incommingFD, int socketFD, sockaddr_in addr) // Sets up the fd
{
    // Create connection, get header from request
    Connection connection;
    connection.fd = incommingFD;
    connection.connectTime = std::time(nullptr);
    connection.request = HttpRequest(incommingFD, config.getRequestSizeLimit());
    connection.addr = addr;

    for (std::list<std::pair<int, bool>>::iterator it = serverSocketFDS.begin(); it != serverSocketFDS.end(); ++it)
    {
        if (it->first == socketFD)
        {
            connection.socketFD = socketFD;
            it->second = true;
        }
    }

    connections.push_back(connection);

    std::cout << "Connected " << connection.fd << " to server\n";
}

void Server::disconnect(std::list<Connection>::iterator connectionIT)
{
    std::cout << "Disconnecting " << connectionIT->fd << " from server\n";

    for (std::list<std::pair<int, bool>>::iterator it = serverSocketFDS.begin(); it != serverSocketFDS.end(); ++it)
    {
        if (it->first == connectionIT->socketFD)
        {
            it->second = false;
        }
    }
    connections.erase(connectionIT);
}

void Server::getRequest(int fd)
{
    std::list<Connection>::iterator it;
    for (it = connections.begin(); it != connections.end(); ++it)
    {
        if (it->fd == fd)
            break;
    }

	it->readAttempted = true;

    if (!it->request.isComplete())
    {
        std::cout << "\n--- Reading request ---\n";
        it->request.readRequest();
    }
}

std::string Server::newSessionId()
{
    std::stringstream stream;
    std::numeric_limits<unsigned long long> ulonglonglim;
    std::random_device rd;  // a seed source for the random number engine
    std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<unsigned long long> distrib(0, ulonglonglim.max());
    stream << std::hex << distrib(gen);
    return stream.str();
}

std::pair<bool, ServerResponse> Server::respond(int fd)
{
    ServerResponse res;
    std::list<Connection>::iterator it;
    for (it = connections.begin(); it != connections.end(); ++it)
    {
        if (it->fd == fd)
            break;
    }

	if (!it->readAttempted)
		return (std::pair<bool, ServerResponse>(false, res));

    bool requestComplete = it->request.isComplete();

    if (requestComplete)
    {
		// Only find route if no previous error occured
        if (it->request.getFailResponseCode() == 0)
        {
            std::cout << "\n--- Finding route ---\n";
            try
            {
                it->route = findCorrectRoute(it->request);
				if (it->route.redirect != "")
					it->request.setFailResponseCode(307);
            }
            catch(const std::exception& e)
            {
                std::cout << "Server: FindCorrectRouteError: " << e.what() << "\n";
                it->request.setFailResponseCode(404);
            }
        }

        if (it->request.getFailResponseCode() != 0 || it->request.isCgi() == false)
        {
            std::cout << "\n--- Responding to client ---\n";
            std::string sessionIndex;
            if (config.hasSessions())
            {
                // timeout sessions, not sure if this point in the code is the best for it
                /* for (std::list<Session>::iterator s = sessions.begin(); s != sessions.end();) */
                /* { */
                /*     if (s->timeout < std::time(nullptr)) */
                /*         s = sessions.erase(s); */
                /*     else */
                /*         s++; */
                /* } */
                std::string cookies = it->request.getHeader("cookie");
                if (cookies != "")
                {
                    size_t pos = cookies.find("session=");
                    if (pos != std::string::npos)
                    {
                        size_t end = cookies.find(';', pos);
                        std::string sessionid = cookies.substr(pos + 8, end == std::string::npos ? end : cookies.length()).c_str();
                        std::list<Session>::iterator session = std::find_if(
                            sessions.begin(), sessions.end(),
                            [&](Session s){return s.sessionid == sessionid;});
                        if (session == sessions.end())
                            it->request.setFailResponseCode(401);
                        if (session->timeout < std::time(nullptr))
                        {
                            it->request.setFailResponseCode(419);
                            sessions.erase(session);
                        }
                        else
                            sessionIndex = sessionid;
                    }
                    else
                    {
                        std::string sessionid = newSessionId();
                        sessions.push_back((Session){sessionid, std::time(nullptr) + config.getSessionTimeout()});
                        sessionIndex = sessionid;
                    }
                }
                else
                {
                    std::string sessionid = newSessionId();
                    sessions.push_back((Session){sessionid, std::time(nullptr) + config.getSessionTimeout()});
                    sessionIndex = sessionid;
                }
            }
            HttpResponse response(it->request, it->route, config.getErrorPage(), config.hasSessions(), sessionIndex);
			ssize_t ret = send(fd, &response.getResponse()[0], response.getResponse().size(), 0);
			// On failed send, disconnect as with successfull send
            if (ret == -1)
                std::cerr << "Failed to send response\n";
			if (ret == 0)
				std::cerr << "Sent empty response\n";
			disconnect(it);
        }
        else
        {
            std::cout << "\n--- Running CGI ---\n";
            try
            {
                std::pair<int, int> ret = cgi_handler.runCGI(it->request, config, it->addr, it->route);
                disconnect(it);
                res.pid = ret.first;
                res.fd = ret.second;
            }
            catch(const std::exception &e)
            {
                it->request.setFailResponseCode(403);
                std::cerr << "RunCGI Error: " << e.what() << std::endl;
				// Returns false to indicate that the response was not sent
				// will be sent in next epoll loop
				return (std::pair<bool, ServerResponse>(false, res));
            }
        }
    }
    return (std::pair<bool, ServerResponse>(requestComplete, res));
}

std::vector<int>    Server::clearTimedOut(void)
{
    std::vector<int> timedOutFDS;

    try
    {
        for (std::list<Connection>::iterator it = connections.begin(); it != connections.end(); ++it)
        {
            if (it->timedOut)
            {
                timedOutFDS.push_back(it->fd);
                disconnect(it);
				it = connections.begin();
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "clearTimedOut Error: " << e.what() << std::endl;
    }
    return timedOutFDS;
}

void    Server::checkTimeouts(void)
{
    try
    {
        std::time_t currentTime = std::time(nullptr);
        for (std::list<Connection>::iterator it = connections.begin(); it != connections.end(); ++it)
        {
            if (currentTime - it->connectTime >= config.getConnectionTimeout())
            {
                std::cout << "Connection timed out on fd " << it->fd << "\n";
                it->request.setFailResponseCode(408);
                it->timedOut = true;
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "checkTimeouts Error: " << e.what() << std::endl;
    }
}

bool	Server::requestComplete(int fd)
{
	std::list<Connection>::iterator it;
	for (it = connections.begin(); it != connections.end(); ++it)
	{
		if (it->fd == fd)
			break;
	}

	return it->request.isComplete();
}
