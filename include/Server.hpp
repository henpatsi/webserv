#ifndef SERVER_HPP
#define SERVER_HPP

#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

#include <list>
#include <algorithm>

struct Connection
{
    int fd;
    Route route;
    HttpRequest request;
    bool headerRead = false;
};

class Server {
private:
    // sockets filedescriptor used for connecting to us
    std::list<std::pair<int, bool>> serverSocketFDS;
    // fd created on accepting a connection
    std::list<Connection> listeningFDS;
    // contains all info and routes about the server
    ServerConfig config;

    // private function to construce answer for public method respond
    std::string GetAnswer();
    // utilfunction of getAnswer, finds the route that matches request location
    Route findCorrectRoute(HttpRequest request);
public:
    Server(ServerConfig config);

    /* ---- Getters ---- */
    int     IsServerSocketFD(int fd)    { return std::any_of(serverSocketFDS.begin(), serverSocketFDS.end(), [&](std::pair<int, bool> x){return x.first == fd && !x.second; }); }
    bool    IsListeningFD(int fd)       { return std::any_of(listeningFDS.begin(), listeningFDS.end(), [&](Connection x){return x.fd == fd; });}
    std::list<std::pair<int, bool>> GetSocketFDs() { return serverSocketFDS; }; 
    // method called on incomming request 
    void connect(int incommingFD, int socketFD);
    // TODO method called when request is done
    // gets called when server can respond 
    bool respond(int fd);

    class SocketOpenException : public std::exception {
        const char * what() const noexcept { return ("Couldnt open socket"); }
    };
    class SocketOptionException : public std::exception {
        const char * what() const noexcept { return ("Failed to set socket option"); }
    };
    class BindException : public std::exception {
        const char * what() const noexcept { return ("Failed to bind"); }
    };
    class ListenException : public std::exception {
        const char * what() const noexcept { return ("Failed to listen"); }
    };
    class RouteException : public std::exception {
        const char * what() const noexcept { return ("Failed to find route"); }
    };
};

#endif
