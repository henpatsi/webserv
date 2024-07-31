#ifndef SERVER_HPP
#define SERVER_HPP

#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

#include <list>
#include <algorithm>

class Server {
private:
    // sockets filedescriptor used for connecting to us
    std::list<std::pair<int, bool>> serverSocketFDS;
    // fd created on accepting a connection
    std::list<std::pair<int, HttpRequest>> listeningFDS;
    // contains all info and routes about the server
    ServerConfig config;
    // TODO used to store chuncked request and build one as soon as its done
    HttpRequest currentRequest;

    // private function to construce answer for public method respond
    std::string GetAnswer();
    // utilfunction of getAnswer, finds the route that matches request location
    Route findCorrectRoute(HttpRequest request);
public:
    Server(ServerConfig config);

    /* ---- Getters ---- */
    int     IsServerSocketFD(int fd)    { return std::any_of(serverSocketFDS.begin(), serverSocketFDS.end(), [&](std::pair<int, bool> x){return x.first == fd && !x.second; }); }
    bool    IsListeningFD(int fd)       { return std::any_of(listeningFDS.begin(), listeningFDS.end(), [&](std::pair<int, HttpRequest> x){return x.first == fd; });}
    std::list<std::pair<int, bool>> GetSocketFDs() { return serverSocketFDS; }; 
    // method called on incomming request 
    void connect(int incommingFD, int socketFD);
    // TODO method called when request is done
    // gets called when server can respond 
    bool respond(int fd);

    class SocketOpenException : std::exception {
        const char * what() const noexcept { return ("Couldnt open socket"); }
    };
    class SocketOptionException : std::exception {
        const char * what() const noexcept { return ("Failed to set socket option"); }
    };
    class BindException : std::exception {
        const char * what() const noexcept { return ("Failed to bind"); }
    };
    class ListenException : std::exception {
        const char * what() const noexcept { return ("Failed to listen"); }
    };
};

#endif
