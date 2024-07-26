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
};


#endif