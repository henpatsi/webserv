#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <list>

#define SPACECHARS " \f\n\r\t\v"

struct Route {
    std::string location = nullptr;
    bool allowedGet = false;
    bool allowedPost = false;
    bool allowedDelete = false;
    std::string redirect = nullptr;
    std::string root = nullptr;
    bool directoryListing = false;
    std::string defaultAnswer = nullptr;
    std::string CGI = nullptr;
    bool acceptUpload = false;
    std::string uploadDir = nullptr;
};

class ServerConfig {
    private:
        std::string _name = nullptr;
        bool _isNameSet = false;
        u_int16_t _port = 8080;
        bool _isPortSet = false;
        struct sockaddr_in _address;
        bool _isAddressSet = false;
        long _clientBodyLimit = 1024;
        bool _isRequestSizeSet = false;
        std::list<struct Route> _routes; // list of Routes with location and all the info
        bool _isRouteSet = false;
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
        class InvalidValueException : std::exception
        {
            private:
                std::string _key;
            public:
                InvalidValueException(std::string key);
                const char *what() const noexcept;
        };
        class MissingLocationException : std::exception 
        {
                const char *what() const noexcept;
        };
        /* ---- Parsers ---- */
        /* ---- ServerParser ----*/
        void parseName(std::string pair, std::string key);
        void parsePort(std::string pair, std::string key);
        void parseAddress(std::string pair, std::string key);
        void parseRoute(std::string pair, std::string key);
        void parseRequestSize(std::string pair, std::string key);
        /* ---- RouteParser ----*/
        void parseAllowedMethods(std::string pair, std::string key, Route& res);
        void parseRedirect(std::string pair, std::string key, Route& res);
        void parseRoot(std::string pair, std::string key, Route& res);
        void parseDirListing(std::string pair, std::string key, Route& res);
        void parseDefaultAnswer(std::string pair, std::string key, Route& res);
        void parseCGI(std::string pair, std::string key, Route& res);
        void parseAcceptUpload(std::string pair, std::string key, Route& res);
        void parseUploadDir(std::string pair, std::string key, Route& res);
        static unsigned int convertIP(std::string ip);
};

#endif