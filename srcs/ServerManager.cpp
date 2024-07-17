#include "ServerManager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>

ServerManager::ServerManager(const std::string path) : _path(path)
{
    // keeps track of the stringstreams for each server
    std::vector<std::stringstream *> configs;
    int x = 0;
    // utils for locations to have { } and for error handling
    bool is_in_server = false;
    int depth = 0;

    // checks if file is openable
    std::ifstream config{ _path };
    if (!config.is_open())
        throw FileIssueException();

    // reads the file into a stringstream
    for (std::string line; std::getline(config, line); )
    {
        // ignores lines starting with #
        size_t firstIndex = line.find_first_not_of(" \f\n\r\t\v");
        if (firstIndex < line.length() && line[firstIndex] != '#')
        {
            // if we find {
            size_t open_index = line.find('{');
            if (open_index != std::string::npos)
            {
                // check if we are creating a new server
                // should check if anything between server and {
                if (line.starts_with("server"))
                {
                    if (is_in_server)
                        throw ServerInServerException();
                    // start this servers config stringstream
                    std::stringstream* s = new std::stringstream();
                    configs.push_back(s);
                    std::cout << "num of servers " << x++ << "\n";
                    is_in_server = true;
                }
                depth += 1;
            }
            size_t close_index = line.find('}');
            if (close_index != std::string::npos)
            {
                // if we are not in parenthesies, or we just got into them but close before
                if (depth == 0)
                    throw InvalidBraceException();
                else
                    depth--;
            }
            if (depth == 0)
                is_in_server = false;
            if (is_in_server) // remove tailing }
                *configs.back() << line;
        }
    }
    config.close();
    if (depth != 0)
        throw UnclosedBraceException();

    // Initialises the Serverconfigs with the correct string
    bool success = true;
    for (std::stringstream* config : configs)
    {
        try
        {
            ServerConfig current(*config);
            servers.push_back(new Server(current));
        }
        catch (const std::exception& e)
        {
            success = false;
            std::cerr << e.what() << "\n";
        }
    }
    if  (!success)
    {
        throw ServerCreationException();
    }
}

ServerManager::~ServerManager()
{
    // deletes all the servers managed by it
}

std::string ServerManager::GetPath() const
{
    return (_path);
}

const char * ServerManager::FileIssueException::what() const noexcept
{
    return "Manager: FileError: Failed to open file";
}

const char * ServerManager::ServerInServerException::what() const noexcept
{
    return "Manager: LogicError: Server inside Server";
}

const char * ServerManager::InvalidBraceException::what() const noexcept
{
    return "Manager: ParsingError: invalid '}'";
}

const char * ServerManager::CharOutsideServerBlockException::what() const noexcept
{
    return "Manager: ParsingError: Character outside server block";
}

const char * ServerManager::ServerCreationException::what() const noexcept
{
    return "Manager: ParsingError: Failed to create Servers";
}

const char * ServerManager::UnclosedBraceException::what() const noexcept
{
    return "Manager: ParsingError: Unclosed brace";
}

void ServerManager::runServers()
{
    // arbitrary max epoll size 50
    int polled = epoll_create1(50);
    struct epoll_event ev;

    ev.events = EPOLLIN | EPOLLET;
    for (auto& server : servers)
    {
        if (epoll_ctl(polled, EPOLL_CTL_ADD, server->connectionSocketFD, &ev) < 0)
            std::cout << "Error while polling fds\n";
        else
            std::cout << "inserted fd\n";
    }
    struct epoll_event *events = new epoll_event[50];
    int currentFds = 1;
    int numberOfEvents;

    sockaddr_storage client_addr;
    int addressSize = sizeof(client_addr);

    int incommingFD;
    while (1)
    {
        numberOfEvents = epoll_wait(polled, events, currentFds, -1);
        if (numberOfEvents == -1)
        {
            std::cout << "epoll issue\n";
            break;
        }
        for (int i = 0; i < numberOfEvents; i++)
        {
            for (auto& server : servers)
            {
                // if they are trying to connect to us
                if (events[i].data.fd == server->connectionSocketFD)
                {
                    // create the incomming filedescriptor
                    incommingFD = accept(events[i].data.fd, (sockaddr*)&client_addr, (socklen_t*)&addressSize);
                    if (incommingFD == -1)
                        std::cout << "Error while accepting message\n";
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = incommingFD;
                    // add it to the polled fds
                    if (epoll_ctl(polled, EPOLL_CTL_ADD, incommingFD, &ev) < 0)
                        std::cout << "Error while adding fds to the epoll\n";
                    // keep track of how many we are managing
                    currentFds++;
                    // read the incomming request into the servers current httprequest
                    std::string requestString;
                    int bufferSize = server->config.getRequestSizeLimit();
                    char messageBuffer[bufferSize + 1];
                    messageBuffer[bufferSize] = '\0';
                    // todo the chunked requests
                    int readAmount = read(incommingFD, messageBuffer, bufferSize);
                    if (readAmount == -1)
                        std::cout << "Error while reading request\n";
                    requestString += messageBuffer; 
                    server->currentRequest = new HttpRequest(requestString);
                }
                // if we can send them data and resolve the request
                else
                {
                    // get the appropriate answer from the server
                    std::string response = server->GetAnswer();
                    // send the answer and check for success
                    if (send(events[i].data.fd, response.c_str(), response.length(), 0) == -1)
                        std::cout << "Error while sending message\n";
                    // remove the fd from the polled fds
                    if (epoll_ctl(polled, EPOLL_CTL_DEL, events[i].data.fd, &ev) < 0)
                        std::cout << "Error while removing filedescriptor from poll\n";
                    currentFds--;
                    // close the fd
                    close(events[i].data.fd);
                    // not sure if this can be added, but would be nice to hard remove it 
                    events[i].data.fd = -1;
                }
            }
        }
    }
    

}