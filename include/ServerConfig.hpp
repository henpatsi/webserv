#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <list>
#include <vector>

#define SPACECHARS " \f\n\r\t\v"

#ifndef DEBUG
# define DEBUG 0
#endif

struct Route {
	std::string	location;
	uint8_t		allowedMethods = 0;
	std::string	redirect;
	std::string	root;
	bool		directoryListing = false;
	std::string	defaultAnswer;
	std::string	CGI;
	bool		acceptUpload = false;
	std::string	uploadDir;
};

class ServerConfig {
	private:
		std::string                     _name;
		bool                            _isNameSet			= false;
		std::vector<u_int16_t>          _ports;
		bool                            _isPortSet			= false;
		unsigned int					_ip;
		std::vector<struct sockaddr_in> _addresses;
		bool                            _isAddressSet		= false;
		long                            _clientBodyLimit	= 1024;
		bool							_isRequestSizeSet 	= false;
		std::list<struct Route>			_routes;
		bool							_isRouteSet			= false;
		int								_connectionTimeout	= 10;
		bool							_isConnectionTimeoutSet = false;
		std::string						_errorPage;
		bool							_isErrorPageSet		= false;
		int								_sessionTimeout;
		bool							_isSessionTimeoutSet= false;

	public:
		ServerConfig(std::stringstream& config);
		~ServerConfig() {};

		/* ---- Getters ----*/
		std::string 					getName() 				{ return _name; };
		std::vector<u_int16_t> 			getPorts() 				{ return _ports; };
		std::vector<struct sockaddr_in>	getAddress() 			{ return _addresses; };
		long 							getRequestSizeLimit() 	{ return _clientBodyLimit; };
		std::list<struct Route> 		getRoutes()				{ return _routes; };
		int								getConnectionTimeout()	{ return _connectionTimeout; };
		std::string						getErrorPage()			{ return _errorPage; };
		int								getSessionTimeout()		{ return _sessionTimeout; };
		bool							hasSessions()			{ return _isSessionTimeoutSet; };

		/* ---- Exceptions ----*/
		class SameKeyRepeatException : public std::exception
		{
			private:
				std::string _key;
			public:
				SameKeyRepeatException(std::string key) : _key("SameKeyRepeatException " + key) {};
				const char *what() const noexcept { return _key.c_str(); };
		};
		class InvalidKeyException : public std::exception
		{
			private:
				std::string _key;
			public:
				InvalidKeyException(std::string key) : _key("InvalidKeyException " + key) {};
				const char *what() const noexcept { return _key.c_str(); };
		};
		class InvalidValueException : public std::exception
		{
			private:
				std::string _key;
			public:
				InvalidValueException(std::string key) : _key("InvalidValueException " + key) {};
				const char *what() const noexcept { return _key.c_str(); };
		};
		class MissingValueException : public std::exception 
		{
			private:
				std::string _key;
			public:
				MissingValueException(std::string key) : _key("MissingValueException " + key) {};
				const char *what() const noexcept { return _key.c_str(); };
		};
		
		/* ---- Parsers ---- */
		/* ---- ServerParser ----*/
		void parseName(std::string pair, std::string key);
		void parsePort(std::string pair, std::string key);
		void parseAddress(std::string pair, std::string key);
		void parseRoute(std::string pair, std::string key);
		void parseRequestSize(std::string pair, std::string key);
		void parseConnectionTimeout(std::string pair, std::string key);
		void parseErrorPage(std::string pair, std::string key);
		void parseSessionTimeout(std::string pair, std::string key);

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
		static uint8_t		parseRequestMethod(std::string s);
};

#endif
