#include "ServerConfig.hpp"
#include <vector>
#include <utility> 
#include <algorithm>
#include <functional>
#include <arpa/inet.h> 
#include <iostream>

struct field {
    std::string name;
    void (ServerConfig::*parse)(std::string, std::string);
};

struct routeField {
    std::string name;
    void (ServerConfig::*parse)(std::string, std::string, Route&);
};

ServerConfig::ServerConfig(std::stringstream& config)
{
    _ports      = std::vector<u_int16_t>();
    _addresses  = std::vector<struct sockaddr_in>();
    _routes     = std::list<Route>();
    std::string _;
    std::getline(config, _);
	if (DEBUG)
	{
		std::cout << _ << "\n";
		std::cout << "full config:<" << config.str() << ">\n";
	}
    std::vector<field> fields {{
        (field){"name:", &ServerConfig::parseName},
        (field){"port:", &ServerConfig::parsePort},
        (field){"host:", &ServerConfig::parseAddress},
        (field){"size_limit:", &ServerConfig::parseRequestSize},
        (field){"location", &ServerConfig::parseRoute},
        (field){"connection_timeout:", &ServerConfig::parseConnectionTimeout},
        (field){"error_page:", &ServerConfig::parseErrorPage},
        (field){"session_timeout:", &ServerConfig::parseSessionTimeout}
    }};
    for (std::string key_value_pair; std::getline(config, key_value_pair, '\n');)
    {
        if (key_value_pair.find('{') != std::string::npos)
        {
            for (std::string extra; std::getline(config, extra);)
            {
                key_value_pair += extra + "\n";
                if (extra.find("}") != std::string::npos)
                    break;
            }
        }
        key_value_pair.erase(0, key_value_pair.find_first_not_of(SPACECHARS));
        std::vector<field>::iterator it = std::find_if(
            fields.begin(), fields.end(),
            [&](field f){return key_value_pair.compare(0, f.name.length(), f.name) == 0;}
        );
        if (it == fields.end())
            throw InvalidKeyException(key_value_pair);
        try
        {
            (this->*it->parse)(key_value_pair, it->name);
        }
        catch (const std::exception& e)
        {
            std::cerr << "ServerConfig: ParseException: " << e.what() << "\n" ;
        }
    }

    if (_isAddressSet == false)
        throw MissingValueException("Address");
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = _ip;
    for (auto& port : _ports)
    {
        addr.sin_port = port;
        _addresses.push_back(addr);
    }
    if (_addresses.empty())
        throw MissingValueException("Address");

    if (_routes.size() == 0)
        throw MissingValueException("Route");

    if (_isNameSet == false)
        throw MissingValueException("Name");
}

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
    size_t index = pair.find_first_not_of(SPACECHARS, key.length());
    if (index == std::string::npos)
        throw InvalidValueException("port");
    std::string s = pair.substr(pair.find_first_not_of(SPACECHARS, key.length()));
    _ports.push_back(htons(std::atol(s.c_str())));
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
    _ip = convertIP(s);
    _isAddressSet = true;
}

void ServerConfig::parseRequestSize(std::string pair, std::string key)
{
    if (_isRequestSizeSet)
        throw SameKeyRepeatException("size_limit");
    std::string s = pair.substr(pair.find_first_not_of(SPACECHARS, key.length()));
    _clientBodyLimit = std::atol(s.c_str());
    _isRequestSizeSet = true;
}

void ServerConfig::parseConnectionTimeout(std::string pair, std::string key)
{
    if (_isConnectionTimeoutSet)
        throw SameKeyRepeatException("connection_timeout");
    std::string s = pair.substr(pair.find_first_not_of(SPACECHARS, key.length()));
    _connectionTimeout = std::atoi(s.c_str());
    _isConnectionTimeoutSet = true;
}

void ServerConfig::parseErrorPage(std::string pair, std::string key)
{
    if (_isErrorPageSet)
        throw SameKeyRepeatException("errorpage");
    std::string s = pair.substr(pair.find_first_not_of(SPACECHARS, key.length()));
    _errorPage = s;
    _isErrorPageSet = true;
}

void ServerConfig::parseSessionTimeout(std::string pair, std::string key)
{
    if (_isSessionTimeoutSet)
        throw SameKeyRepeatException("session_timeout");
    std::string s = pair.substr(pair.find_first_not_of(SPACECHARS, key.length()));
    _sessionTimeout = std::atoi(s.c_str());
    _isSessionTimeoutSet = true;
}

void ServerConfig::parseRoute(std::string pair, std::string key)
{
    std::string s = pair.substr(pair.find_first_not_of(SPACECHARS, key.length()));
    size_t openIndex = s.find('{');
    size_t closeIndex = s.find('}');
    if (openIndex == std::string::npos || openIndex == 0 || closeIndex == std::string::npos || openIndex >= closeIndex)
        throw InvalidValueException("Route");
    std::string route = s.substr(0, openIndex - 1);
    std::stringstream routeContent(s.substr(openIndex + 1, closeIndex - openIndex - 2));
    Route res;
    res.location = route;
    std::cout << route << "\n";
    _isRouteSet = true;
    std::vector<routeField> parsers {{
        (routeField){"allowedMethods:", &ServerConfig::parseAllowedMethods},
        (routeField){"redirect:", &ServerConfig::parseRedirect},
        (routeField){"root:", &ServerConfig::parseRoot},
        (routeField){"dirListing:", &ServerConfig::parseDirListing},
        (routeField){"index:", &ServerConfig::parseDefaultAnswer},
        (routeField){"CGI:", &ServerConfig::parseCGI},
        (routeField){"acceptUpload:", &ServerConfig::parseAcceptUpload},
        (routeField){"uploadDir:", &ServerConfig::parseUploadDir}
    }};
    for (std::string keyvaluepair; std::getline(routeContent, keyvaluepair);)
    {
        keyvaluepair.erase(0, keyvaluepair.find_first_not_of(SPACECHARS));
        if (keyvaluepair == "")
            continue;
        std::vector<routeField>::iterator it = std::find_if(
            parsers.begin(), parsers.end(),
            [&](routeField f){return keyvaluepair.compare(0, f.name.length(), f.name) == 0;}
        );
        if (it == parsers.end())
            throw ServerConfig::InvalidKeyException(keyvaluepair);
        try
        {
            (this->*it->parse)(keyvaluepair, it->name, res);
        }
        catch (const std::exception& e)
        {
            std::cerr << "ServerConfig: ParseException: RouteException: " << e.what() << "\n" ;
            throw e;
        }
    }

    if (res.location == "")
        throw InvalidValueException("location");

    if (res.allowedMethods == 0 && res.redirect == "")
        throw InvalidValueException("allowed methods");

    if (res.root == "")
    {
        if (res.redirect == "")
            throw InvalidValueException("root");
    }

    if (res.uploadDir != "" && res.acceptUpload == false)
        throw InvalidValueException("Upload");

    _routes.push_back(res);
    _isRouteSet = true;
}

/* RouteParsers */
void ServerConfig::parseAllowedMethods(std::string pair, std::string key, Route& res)
{
    size_t index = pair.find_first_not_of(SPACECHARS, key.length());
    if (index == std::string::npos)
        throw InvalidValueException("method");
    std::string s = pair.substr(pair.find_first_not_of(SPACECHARS, key.length()));
    if (s.find("POST") != std::string::npos)
        res.allowedMethods |= parseRequestMethod("POST");
    if (s.find("GET") != std::string::npos)
        res.allowedMethods |= parseRequestMethod("GET");
    if (s.find("DELETE") != std::string::npos)
        res.allowedMethods |= parseRequestMethod("DELETE");
    if (s.find("HEAD") != std::string::npos)
        res.allowedMethods |= parseRequestMethod("HEAD");
}

void ServerConfig::parseRedirect(std::string pair, std::string key, Route& res)
{
    size_t index = pair.find_first_not_of(SPACECHARS, key.length());
    if (index == std::string::npos)
        throw InvalidValueException("redirect");
    std::string s = pair.substr(pair.find_first_not_of(SPACECHARS, key.length()));
    res.redirect = s;
}

void ServerConfig::parseRoot(std::string pair, std::string key, Route& res)
{
    size_t index = pair.find_first_not_of(SPACECHARS, key.length());
    if (index == std::string::npos)
        throw InvalidValueException("webroot");
    std::string s = pair.substr(pair.find_first_not_of(SPACECHARS, key.length()));
    res.root = s;
}

void ServerConfig::parseDirListing(std::string pair, std::string key, Route& res)
{
    size_t index = pair.find_first_not_of(SPACECHARS, key.length());
    if (index == std::string::npos)
        throw InvalidValueException("Directory listing");
    std::string s = pair.substr(pair.find_first_not_of(SPACECHARS, key.length()));
    if (s == "true")
        res.directoryListing = true;
    else if (s == "false")
        res.directoryListing = false;
    else
        throw InvalidValueException("Directory listing");
}

void ServerConfig::parseDefaultAnswer(std::string pair, std::string key, Route& res)
{
    size_t index = pair.find_first_not_of(SPACECHARS, key.length());
    if (index == std::string::npos)
        throw InvalidValueException("Default Answer");
    std::string s = pair.substr(pair.find_first_not_of(SPACECHARS, key.length()));
    res.defaultAnswer = s;
}

void ServerConfig::parseCGI(std::string pair, std::string key, Route& res)
{
    size_t index = pair.find_first_not_of(SPACECHARS, key.length());
    if (index == std::string::npos)
        throw InvalidValueException("Uploaddir");
    std::string s = pair.substr(pair.find_first_not_of(SPACECHARS, key.length()));
    res.CGI = s;
}

void ServerConfig::parseAcceptUpload(std::string pair, std::string key, Route& res)
{
    size_t index = pair.find_first_not_of(SPACECHARS, key.length());
    if (index == std::string::npos)
        throw InvalidValueException("Accept Upload");
    std::string s = pair.substr(pair.find_first_not_of(SPACECHARS, key.length()));
    if (s == "true")
        res.acceptUpload = true;
    else if (s == "false")
        res.acceptUpload = false;
    else
        throw InvalidValueException("Accept Upload");
}

void ServerConfig::parseUploadDir(std::string pair, std::string key, Route& res)
{
    size_t index = pair.find_first_not_of(SPACECHARS, key.length());
    if (index == std::string::npos)
        throw InvalidValueException("Uploaddir");
    std::string s = pair.substr(pair.find_first_not_of(SPACECHARS, key.length()));
    res.uploadDir = s;
}


unsigned int ServerConfig::convertIP(std::string ip)
{
    std::stringstream s(ip);
    std::string val;
    unsigned int ip_long = 0;
    for (int i = 3; i >= 0 && std::getline(s, val, '.'); i--)
    {
        unsigned int x = std::atoi(val.c_str());
        if (x > 255)
        {
            throw InvalidValueException("ip");
        }
        ip_long += x << ((3 - i) * 8);
    
    }
    return ip_long;
}

uint8_t ServerConfig::parseRequestMethod(std::string s)
{
    if (s == "GET")
        return 1 << 1;
    if (s == "POST")
        return 1 << 2;
    if (s == "DELETE")
        return 1 << 3;
    if (s == "HEAD")
        return 1 << 4;
    return 0;
}
