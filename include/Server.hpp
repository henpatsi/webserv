#ifndef SERVER_HPP
#define SERVER_HPP

#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

class Server {
private:
    // sockets filedescriptor used for connecting to us
    int serverSocketFD;
    // fd created on accepting a connection
    int listeningFD;
    // contains all info and routes about the server
    ServerConfig config;
    // TODO used to store chuncked request and build one as soon as its done
    HttpRequest *currentRequest;

    // private function to construce answer for public method respond
    std::string GetAnswer();
    // utilfunction of getAnswer, finds the route that matches request location
    Route findCorrectRoute(HttpRequest request);
public:
    Server(ServerConfig config);

    /* ---- Getters ---- */
    int GetServerSocketFD() { return serverSocketFD; }
    int GetListeningFD() { return listeningFD; }

    // method called on incomming request 
    void connect(int incommingFD);
    // TODO method called when request is done
    // gets called when server can respond 
    void respond();

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