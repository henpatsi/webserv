#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <list>

#define SPACECHARS " \f\n\r\t\v"

struct Route {
    uint8_t allowedMethods;
    std::string redirect;
    std::string root;
    bool directoryListing;
    std::string defaultAnswer;
    std::string CGI;
    bool acceptUpload;
    std::string uploadDir;
};

/* Small redsign idea making all fields the same type of struct with default value, parser, template ...*/

class ServerConfig {
    private:
        std::string _name;
        bool _isNameSet;
        u_int16_t _port;
        bool _isPortSet;
        struct sockaddr_in _address;
        bool _isAddressSet;
        long _clientBodyLimit;
        bool _isRequestSizeSet;
        std::list<struct Route> _routes; // list of Routes with location and all the info
        bool _isRouteSet;
    public:
        ServerConfig(std::stringstream& config);
        ~ServerConfig();
        /* ---- Getters ----*/
        std::string getName();
        u_int16_t getPort();
        struct sockaddr_in getAddress();
        long getRequestSizeLimit();
        std::list<struct Route> getRoutes();
        /* ---- Exceptions ----*/
        class SameKeyRepeatException : std::exception
        {
            private:
                std::string _key;
            public:
                SameKeyRepeatException(std::string key);
                const char *what() const noexcept;
        };
        class InvalidKeyException : std::exception
        {
            private:
                std::string _key;
            public:
                InvalidKeyException(std::string key);
                const char *what() const noexcept;
        };
        class MissingLocationException : std::exception {
                const char *what() const noexcept;
        };
        /* ---- Parsers ---- */
        void parseName(std::string pair, std::string key);
        void parsePort(std::string pair, std::string key);
        void parseAddress(std::string pair, std::string key);
        void parseRoute(std::string pair, std::string key);
        void parseRequestSize(std::string pair, std::string key);
        static unsigned int convertIP(std::string ip)
};

#endif