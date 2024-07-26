#include "ServerManager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>



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
    // setup epoll
    polled = epoll_create1(EPOLL_SIZE);
    events = new epoll_event[EPOLL_SIZE];
    try{
        registerServerSockets();
    }
    catch(std::exception& e)
    {
        std::cout << e.what();
    }
}

ServerManager::~ServerManager()
{
    // deletes all the servers managed by it
    delete[] events;
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

ServerManager::ManagerRuntimeException::ManagerRuntimeException(std::string error)
{
    this->error = error;
}

const char *ServerManager::ManagerRuntimeException::what() const noexcept
{
    return error.c_str();
}

void ServerManager::runServers()
{
    while (1)
    {
        // waits for at least 1 event to occur
        WaitForEvents();
        for (int i = 0; i < eventAmount; i++)
        {
            for (Server* server : servers)
            {
                // if someone initiates a connection to the registered sockets
                if (events[i].data.fd == server->GetServerSocketFD())
                {
                    try
                    {
                        int incommingFD = acceptConnection(events[i]);
                        AddToEpoll(incommingFD);
                        server->connect(incommingFD);
                    }
                    catch (std::exception& e)
                    {
                        std::cout << e.what() << "\n";
                    }

                }
                // if we can send them data and resolve the request
                else if (events[i].data.fd == server->GetListeningFD())
                {
                    try
                    {
                        server->respond();
                        DelFromEpoll(events[i].data.fd);
                    }
                    catch (std::exception& e)
                    {
                        std::cout << e.what() << "\n";
                    }
                }
            }
        }
    }
}

void ServerManager::AddToEpoll(int fd)
{
    if (epoll_ctl(polled, EPOLL_CTL_ADD, fd, &temp_event) < 0)
        std::cout << "Error while polling fds\n";
    else
        std::cout << "inserted fd\n";
    // keeps the tracked fds for epoll_wait
    trackedFds++;
}

void ServerManager::DelFromEpoll(int fd)
{
    if (epoll_ctl(polled, EPOLL_CTL_DEL, fd, &temp_event) < 0)
        std::cout << "Error while removing fd\n";
    else
        std::cout << "removed fd\n";
    // keeps the tracked fds for epoll_wait
    trackedFds--;
    close(fd);
}

void ServerManager::registerServerSockets()
{
    // incomming connections
    temp_event.events = EPOLLIN | EPOLLET;
    for (Server* server : servers)
    {
        AddToEpoll(server->GetServerSocketFD());
        setFdNonBlocking(server->GetServerSocketFD());
    }
}

void ServerManager::WaitForEvents()
{
    eventAmount = epoll_wait(polled, events, trackedFds, -1);
    if (eventAmount == -1)
        std::cout << "epoll issue\n"; // should be changed to exception
}

int ServerManager::acceptConnection(epoll_event event)
{
    int incommingFd = accept(event.data.fd, (sockaddr*)&client_addr, (socklen_t*)&addressSize);
    if (incommingFd == -1)
        std::cout << "Error\n";
    setFdNonBlocking(incommingFd);
    temp_event.events = EPOLLIN | EPOLLET;
    temp_event.data.fd = incommingFd;
}

static void setFdNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1)
        throw std::system_error();
    flags |= O_NONBLOCK;
    int res = fcntl(fd, F_SETFL, flags);
    if (res == -1)
        throw std::system_error();
}