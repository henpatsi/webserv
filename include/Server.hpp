#ifndef SERVER_HPP
#define SERVER_HPP

#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

class Server {
private:
    int serverSocketFD;
    int listeningFD;
public:
    Server(ServerConfig config);
    HttpRequest currentRequest;
    HttpResponse currentResponse;
    std::string GetAnswer();
    ServerConfig config;

    int GetServerSocketFD() { return serverSocketFD; }
    int GetListeningFD() { return listeningFD; }
    void SetListeningFD(int incommingFd) { listeningFD = incommingFd; }
    Route findCorrectRoute(HttpRequest request);

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
