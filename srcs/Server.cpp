#include "Server.hpp"
#include <iostream>
#include <arpa/inet.h> 
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>

Server::Server(ServerConfig _config) : config(_config)
{
    serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketFD == -1)
        throw SocketOpenException();
    int yes = 1;
	if (setsockopt(serverSocketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
        throw SocketOptionException();
    sockaddr_in serverAddress = config.getAddress();
    if (bind(serverSocketFD, (sockaddr*) &serverAddress, sizeof(serverAddress)) == -1)
		throw BindException();
    if (listen(serverSocketFD, 5) == -1)
		throw ListenException();
    std::cout << "Server listening at addr " << inet_ntoa(serverAddress.sin_addr) << " port " << ntohs(serverAddress.sin_port) << "\n";
}

Route Server::findCorrectRoute(HttpRequest request)
{
    Route fitting;
    bool isFound = false;
    // just checks if resourcePath has the start of the location of the route
    for (Route route : config.getRoutes())
    {
        if (!request.getResourcePath().starts_with(route.location))
            continue;
        // set the current one as matching or replace if the current route matches "more"
        if (!isFound || fitting.location.length() < route.location.length())
            fitting = route;
        isFound = true;
        break;
    }
    if (isFound)
        return fitting;
    else
        throw std::system_error();
}

// constructs response based on the Routes contained in the server, depending on that routes settings
std::string Server::GetAnswer()
{
    std::string result;
    Route selectedRoute;
    try{
        selectedRoute = findCorrectRoute(*currentRequest);
    }
    catch(std::exception& e)
    {
        (void)e;
        // no matching route for request -> get the default errorpage
        return "404 page that has to be made";
    }

    uint8_t requestMethod = ServerConfig::parseRequestMethod(currentRequest->getMethod());
    if (requestMethod & selectedRoute.allowedMethods)
    {
        // check for CGI and redirects
        if (selectedRoute.redirect != "")
        {
            return "302 redirecting to " + selectedRoute.redirect;
        }
        if (selectedRoute.CGI != "")
        {
            // run cgi
            return "500 cgi doing stuff";
        }
        // check for methods
        if (currentRequest->getMethod() == "GET")
        {
            std::string location = currentRequest->getResourcePath();
            location.erase(0, selectedRoute.location.length());
            location += selectedRoute.location;
            if (open(location.c_str(), O_DIRECTORY) != -1)
            {
                if (selectedRoute.directoryListing)
                {
                    // read directory
                    return "200 content of the directory";
                }
                else
                {
                    return "403? cant read directories";
                }
            }
            else
            {
                if (access(location.c_str(), F_OK))
                {
                    if (access(location.c_str(), R_OK))
                        return "200 file found + content";
                    else
                        return "403 no rights for you";
                }
                else
                {
                    return "404 file not found";
                }
            }
        }
        else if (currentRequest->getMethod() == "POST")
        {
            std::string location = currentRequest->getResourcePath();
            location.erase(0, selectedRoute.location.length());
            location += selectedRoute.location;
            // check for simple form
            if (access(location.c_str(), F_OK))
            {
                if (access(location.c_str(), R_OK))
                    return "200 file found + content";
                else
                    return "403 no rights for you";
            }
            if (selectedRoute.acceptUpload && (
                currentRequest->getHeader("x-www-urlencoded") != "" ||
                currentRequest->getHeader("multipart/form-data") != "" ||
                currentRequest->getHeader("text-plain") != ""))
            {
                //check if we have access to uploaddir
                return "200/400 upload stuff or not";
            }
            else
            {
                return "400 cant post stuff here";
            }
        }
        else if (currentRequest->getMethod() == "DELETE")
        {
            // delete if exist or give error if not
            return "200/400 deleting or not";
        }
    }
    else
    {
        return selectedRoute.defaultAnswer == "" ? "404 error page" : selectedRoute.defaultAnswer;
    }
    return "Unknown stuff for now";
}

