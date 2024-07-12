#include "ServerManager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <list>
ServerManager::ServerManager(const std::string path) : _path(path)
{
    // keeps track of the stringstreams for each server
    std::list<std::stringstream> configs;
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
        if (line[line.find_first_not_of(" \f\n\r\t\v")] != '#')
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
                    std::stringstream s("");
                    configs.push_back(s);
                    is_in_server = true;
                }
                depth += 1;
            }
            size_t close_index = line.find('}');
            if (close_index != std::string::npos)
            {
                // if we are not in parenthesies, or we just got into them but close before
                if (depth == 0 || (depth == 1 && close_index < open_index))
                    throw InvalidBraceException();
                else
                    depth--;
            }
            if (depth == 0)
                is_in_server = false;
            if (!is_in_server && line.find_first_not_of(" \f\n\r\t\v", line.find_last_of('}')) != std::string::npos)
                throw CharOutsideServerBlockException();
            configs.back() << line;
        }
    }
    config.close();

    // Initialises the Serverconfigs with the correct string
    bool success = true;
    for (auto& config : configs)
    {
        try
        {
            servers.push_back(new ServerConfig(config));
        }
        catch (const std::exception e)
        {
            success = false;
            std::cerr << "Error: " << e.what() << "\n";
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
