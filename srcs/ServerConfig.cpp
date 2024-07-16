#include "ServerConfig.hpp"
#include <map>
#include <utility> 
#include <algorithm>
#include <functional>
#include <arpa/inet.h> 
#include <iostream>

struct field {
    std::string name;
    void (ServerConfig::*parse)(std::string, std::string);
};

ServerConfig::ServerConfig(std::stringstream& config)
{
    // default route needed?
    // default addess maybe?
    // need to have an extra check for locations, as they have ',' in them
    // needs its separate check
    /* Main server parse */
    // clean config -> remove server {
    std::string _;
    std::getline(config, _, '{');
    std::vector<field> fields ({(field){"name:", &ServerConfig::parseName}, (field){"port:", &ServerConfig::parsePort}, (field){"host:", &ServerConfig::parseAddress},});
    for (std::string key_value_pair; std::getline(config, key_value_pair, ',');)
    {
        key_value_pair.erase(0, key_value_pair.find_first_not_of(SPACECHARS));
        //std::cout << key_value_pair << "\n";
        std::vector<field>::iterator it = std::find_if(fields.begin(), fields.end(), [key_value_pair](field f){return key_value_pair.starts_with(f.name);});
        if (it == fields.end())
            throw InvalidKeyException(key_value_pair);
        try
        {
            (this->*it->parse)(key_value_pair, it->name);
        }
        catch (const std::exception& e)
        {
            throw e;
        }
    }
    if (!_isRouteSet)
        throw MissingLocationException();
    // default address
    if (!_isAddressSet)
    {
        _address.sin_family = AF_INET;
        _address.sin_port = _port;
        _address.sin_addr.s_addr = ServerConfig::convertIP("127.0.0.1");
    }
}

ServerConfig::~ServerConfig(){}

/* ---- Parser Functions ---- */
void ServerConfig::parseName(std::string pair, std::string key)
{
    if (_isNameSet)
        throw SameKeyRepeatException("name");
    size_t index = pair.find_first_not_of(SPACECHARS, key.length());
    if (index == std::string::npos)
        throw InvalidValueException("name");
    _name = pair.substr(index);
    _isNameSet = true;
}

void ServerConfig::parsePort(std::string pair, std::string key)
{
    if (_isPortSet)
        throw SameKeyRepeatException("port");
    size_t index = pair.find_first_not_of(SPACECHARS, key.length());
    if (index == std::string::npos)
        throw InvalidValueException("port");
    std::string s = pair.substr(pair.find_first_not_of(SPACECHARS, key.length()));
    _port = htons(std::atol(s.c_str()));
    _address.sin_port = _port;
    _isPortSet = true;
}

void ServerConfig::parseAddress(std::string pair, std::string key)
{
    if (_isAddressSet)
        throw SameKeyRepeatException("address");
    size_t index = pair.find_first_not_of(SPACECHARS, key.length());
    if (index == std::string::npos)
        throw InvalidValueException("address");
    std::string s = pair.substr(pair.find_first_not_of(SPACECHARS, key.length()));
    _address.sin_family = AF_INET;
    _address.sin_port = _port;
    _address.sin_addr.s_addr = ServerConfig::convertIP(s.c_str());
    _isAddressSet = true;
}

void ServerConfig::parseRequestSize(std::string pair, std::string key)
{
    if (_isRequestSizeSet)
        throw SameKeyRepeatException("port");
    std::string s = pair.substr(pair.find_first_not_of(SPACECHARS, key.length()));
    _clientBodyLimit = std::atol(s.c_str());
    _isRequestSizeSet = true;
}

void ServerConfig::parseRoute(std::string pair, std::string key)
{
    std::string s = pair.substr(pair.find_first_not_of(SPACECHARS, key.length()));
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
    return _key.c_str();
}

ServerConfig::InvalidKeyException::InvalidKeyException(std::string key) : _key(key){}

const char *ServerConfig::InvalidKeyException::what() const noexcept
{
    return _key.c_str();
}

const char *ServerConfig::MissingLocationException::what() const noexcept
{
    return "No location specified";
}

ServerConfig::InvalidValueException::InvalidValueException(std::string key) : _key(key){}

const char *ServerConfig::InvalidValueException::what() const noexcept
{
    return (std::stringstream("ServerConfig: InvalidKey: ") << _key.c_str()).str().c_str();
}

const char *ServerConfig::MissingLocationException::what() const noexcept
{
    return "no location specified";
}

unsigned int ServerConfig::convertIP(std::string ip)
{
    std::stringstream s(ip);
    std::string val;
    unsigned int ip_long = 0;
    for (int i = 4; i >= 1 && std::getline(s, val, '.'); i--)
    {
        //std::cout << "<" << val << ">\n";
        try
        {
            unsigned int x = std::atoi(val.c_str());
            ip_long += x << (i * 8);
        }
        catch(const std::exception& e)
        {
            (void)e;
            return (0);
        }
    }
    // std::cout << ip_long << "\n";
    // std::cout << inet_addr(ip.c_str()) << "\n";
    return ip_long;
}