#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include <string>
#include <vector>
#include "ServerConfig.hpp"
#include "Server.hpp"

#define MANAGEREXCEPTION std::exception { \
            const char * what() const noexcept; \
        }

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

        // connection fields
        const int addressSize = sizeof(sockaddr_storage);
        sockaddr_storage client_addr;
        /*
         * registers and removes filedescriptors to the event poll
         * modify ServerManager.temp_event to get the correct event type
         */
        void AddToEpoll(int fd);
        void DelFromEpoll(int fd);

        void registerServerSockets();
        int acceptConnection(epoll_event event);

        void WaitForEvents();
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
        class ManagerRuntimeException : std::exception {
            private:
                std::string error;
            public:
                ManagerRuntimeException(std::string error);
                const char *what() const noexcept;
        };
        class ManagerEpollException : std::exception {
            private:
                std::string error;
            public:
                ManagerEpollException(std::string error);
                const char *what() const noexcept;
        };
};


#endif