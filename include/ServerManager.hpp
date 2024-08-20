#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include <string>
#include <vector>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include "ServerConfig.hpp"
#include "Server.hpp"

#define CGI_TIMEOUT 2

#define MANAGEREXCEPTION public std::exception { \
            const char * what() const noexcept; \
        }

struct  cgiInfo
{
    int fd;
    int pid;
    cgiResponse response;
    int listeningFd;
    std::time_t cgiStarted;
};

class ServerManager
{
    private:
        std::string _path;
        std::vector<Server *> servers;
        // epoll fields
        const int EPOLL_SIZE = 50;
        int polled;
        struct epoll_event temp_event;
        struct epoll_event* events;
        int trackedFds;
        int eventAmount;
        std::vector <cgiInfo> info;

        // connection fields
        const int addressSize = sizeof(sockaddr_in);
        sockaddr_in client_addr;
        /*
         * registers and removes filedescriptors to the event poll
         * modify ServerManager.temp_event to get the correct event type
         */
        void AddToEpoll(int fd);
        void DelFromEpoll(int fd);

        // registeres all sockets from servers to the epoll
        void registerServerSockets();
        // creates new fd with accept and returns it throws error on accept fail
        int acceptConnection(epoll_event event);

        // sets eventAmount and throws error on epoll_wait error
        void WaitForEvents();
        // checks if any connection has timed out
        void checkTimeouts();

        void makeConnection(Server &server, epoll_event event);
        void readMore(Server &server, epoll_event event);
        void makeResponse(Server &server, epoll_event event);
        void checkCGIFds(epoll_event event);
    void handleCgiResponse(std::vector<cgiInfo>::iterator it);
    public:
        ServerManager(std::string path);
        ~ServerManager();
        
        std::string GetPath() const;

        void runServers();
        
        class FileIssueException : MANAGEREXCEPTION;
        class ServerInServerException : MANAGEREXCEPTION;
        class InvalidBraceException : MANAGEREXCEPTION;
        class CharOutsideServerBlockException : MANAGEREXCEPTION;
        class ServerCreationException : MANAGEREXCEPTION;
        class UnclosedBraceException : MANAGEREXCEPTION;
        class ManagerRuntimeException : public std::exception {
            private:
                std::string error;
            public:
                ManagerRuntimeException(std::string error);
                const char *what() const noexcept;
        };
};


#endif
