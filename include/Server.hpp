#ifndef SERVER_HPP
#define SERVER_HPP

#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "cgiHandler.hpp"

#include <list>
#include <algorithm>
#include <optional>

struct Connection
{
    int fd;
    int socketFD;
    std::time_t connectTime;
    bool isCgi;
    Route route;
    HttpRequest request;
    sockaddr_in addr;
    bool timedOut = false;
	bool readAttempted = false;
};

struct ServerResponse {
    std::optional<int> fd;
    std::optional<int> pid;
};

class Server {
private:
    // sockets filedescriptor used for connecting to us
    std::list<std::pair<int, bool>> serverSocketFDS;
    // fd created on accepting a connection
    std::list<Connection> connections;
    // contains all info and routes about the server
    ServerConfig config;

    // private function to construce answer for public method respond
    std::string GetAnswer();
    // utilfunction of getAnswer, finds the route that matches request location
    Route findCorrectRoute(HttpRequest request);
    cgiHandler  cgi_handler;
public:
    Server(ServerConfig config);

    /* ---- Getters ---- */
    int     IsServerSocketFD(int fd)    { return std::any_of(serverSocketFDS.begin(), serverSocketFDS.end(), [&](std::pair<int, bool> x){return x.first == fd && !x.second; }); }
    bool    IsServerConnection(int fd)       { return std::any_of(connections.begin(), connections.end(), [&](Connection x){return x.fd == fd; });}
    std::list<std::pair<int, bool>> GetSocketFDs() { return serverSocketFDS; }; 
    // method called on incomming request 
    void connect(int incommingFD, int socketFD, sockaddr_in client_address);
    // disconnects when response done or timeout
    void disconnect(std::list<Connection>::iterator connection);
    // gets called when server can read
    void getRequest(int fd);
    // gets called when request read and server can write
    std::pair<bool, ServerResponse> respond(int fd);
    // checks if the connection is still alive
    void checkTimeouts(void);

    std::vector<int> clearTimedOut(void);
	bool	requestComplete(int fd);

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
