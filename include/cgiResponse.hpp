#ifndef CGIRESPONSE_HPP
# define CGIRESPONSE_HPP
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <unistd.h>
#include <HttpResponse.hpp>
# ifndef SPACECHARS
#  define SPACECHARS " \f\n\r\t\v"
# endif

class	cgiResponse
{
	private:
	std::string	_status;
	std::string	_method;
	std::string	_contentType;
	int		_fd;
	int		_reponseCode;
	int		_failResponseCode;
	size_t		_contentLength;
	size_t		_bodyBegin;
	std::map<std::string, std::string>	_headers = {};
	bool		_contentLengthSet;
	bool		_done;
	std::vector<char>	_content;
	char		_buffer[1024];
	std::vector<char>	_message;
	std::string	_messageString;
	std::string	_headerStr;
	void	parseBuffer();
	void	setHeader(std::string line);
	size_t	findBodyStart();
	void	setBodyBegin(std::string &temp);
	void	checkMessageLength();
	void	setContent();
	void	setFields();
	void	checkHeaderStr();
	void	setErrorAndThrow(int responseCode, std::string message);
	public:
	cgiResponse(int fd);
	bool	readCgiResponse(void);
	int	getFailResponseCode();
	std::string	getMethod();
	std::string	getHeaders();
	std::vector<char>	getContent();
	class RequestException : public std::exception
		{
			public:
				RequestException(std::string message) : message(message) {};
				virtual const char* what() const throw();
			private:
				std::string message;
		};

	class ReadException : std::exception {
        const char * what() const noexcept { return ("Read failed"); }
    };
};

#endif
