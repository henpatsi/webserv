#include "ServerConfig.hpp"
#include <map>
#include <utility> 
#include <algorithm>
#include <functional>
#include <arpa/inet.h> 
#include <iostream>

ServerConfig::ServerConfig(std::stringstream& config)
{
    // default route needed?
    // default addess maybe?
    // need to have an extra check for locations, as they have ',' in them
    // needs its separate check
    /* Main server parse */
    std::map<std::string, std::function<void(std::string, std::string)>> mapable;
    mapable.emplace("name:", parseName);
    mapable.emplace("port:", parseName);
    mapable.emplace("host:", parseName);
    mapable.emplace("requstSize:", parseName);
    mapable.emplace("location:", parseName);
    for(std::string line; std::getline(config,line);)
    {
        std::stringstream content(line);
        for (std::string key_value_pair; std::getline(content, key_value_pair, ',');)
        {
            // insanity
            std::map<std::string, std::function<void(std::string, std::string)>>::iterator it = std::find_if(
                mapable.begin(),
                mapable.end(),
                [](std::string whole, std::string key){return whole.starts_with(key);}
            );
            if (it == mapable.end())
                throw InvalidKeyException(key_value_pair);
            it->second(key_value_pair, it->first);
        }
    }

    /* set default values and throw errors */
    if (!_isRouteSet)
        throw MissingLocationException();
    
    if (!_isPortSet)
    {
        _port = 8000;
        _address.sin_port = _port;
    }

    if (!_isAddressSet)
    {
        _address.sin_family = AF_INET;
        _address.sin_port = _port;
        _address.sin_addr.s_addr = inet_addr("127.0.0.1");
    }

    if (!_isRequestSizeSet)
    {
        _clientBodyLimit = 1024;
    }
}

ServerConfig::~ServerConfig(){}

/* ---- Parser Functions ---- */
void ServerConfig::parseName(std::string pair, std::string key)
{
    if (_isNameSet)
        throw SameKeyRepeatException("name");
    _name = pair.substr(key.length() + pair.find_first_not_of(SPACECHARS, key.length()));
    _isNameSet = true;
}

void ServerConfig::parsePort(std::string pair, std::string key)
{
    if (_isPortSet)
        throw SameKeyRepeatException("port");
    std::string s = pair.substr(key.length() + pair.find_first_not_of(SPACECHARS, key.length()));
    _port = htons(std::atol(s.c_str()));
    _address.sin_port = _port;
    _isPortSet = true;
}

void ServerConfig::parseAddress(std::string pair, std::string key)
{
    if (_isAddressSet)
        throw SameKeyRepeatException("address");
    std::string s = pair.substr(key.length() + pair.find_first_not_of(SPACECHARS, key.length()));
    _address.sin_family = AF_INET;
    _address.sin_port = _port;
    _address.sin_addr.s_addr = inet_addr(s.c_str());
    _isAddressSet = true;
}

void ServerConfig::parseRequestSize(std::string pair, std::string key)
{
    if (_isRequestSizeSet)
        throw SameKeyRepeatException("port");
    std::string s = pair.substr(key.length() + pair.find_first_not_of(SPACECHARS, key.length()));
    _clientBodyLimit = std::atol(s.c_str());
    _isRequestSizeSet = true;
}

void ServerConfig::parseRoute(std::string pair, std::string key)
{
    std::string s = pair.substr(key.length() + pair.find_first_not_of(SPACECHARS, key.length()));
    // figure out the route
    // similar to the basic parse
    _isRouteSet = true;
}

/* ---- Getters ----*/

std::string ServerConfig::getName()
{
    return _name;
}

u_int16_t ServerConfig::getPort()
{
    return _port;
}

struct sockaddr_in ServerConfig::getAddress()
{
    return _address;
}

long ServerConfig::getRequestSizeLimit()
{
    return _clientBodyLimit;
}

std::list<struct Route> ServerConfig::getRoutes()
{
    return _routes;
}

/* ---- Exceptions ----*/
ServerConfig::SameKeyRepeatException::SameKeyRepeatException(std::string key) : _key(key){}

const char *ServerConfig::SameKeyRepeatException::what() const noexcept
{
    return (std::stringstream("ServerConfig: SameKeyRepetition: ") << _key.c_str()).str().c_str();
}

ServerConfig::InvalidKeyException::InvalidKeyException(std::string key) : _key(key){}

const char *ServerConfig::InvalidKeyException::what() const noexcept
{
    return (std::stringstream("ServerConfig: InvalidKey: ") << _key.c_str()).str().c_str();
}

static unsigned int ServerConfigs::convertIP(std::string ip)
{
    std::stringstream s(ip);
    std::string val;
    unsigned int ip_long = 0;
    for (int i = 0; i < 4 && std::getline(s, val, '.'); i++)
    {
        try
        {
            char x = std::atoi(val.c_str());
            ip_long += x << (i * 8);
        }
        catch(const std::exception& e)
        {
            return (-1);
        }
        
        
    }
    std::cout << ip_long << "\n";
    std::cout << inet_addr(ip.c_str()) << "\n";
    return ip_long;
}