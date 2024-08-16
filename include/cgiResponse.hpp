#pragma once
#ifndef CGIRESPONSE_HPP
# define CGIRESPONSE_HPP
#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <unistd.h>
# ifndef SPACECHARS
#  define SPACECHARS " \f\n\r\t\v"
# endif
#ifndef MAX_CONTENT_LENGTH
# define MAX_CONTENT_LENGTH 300000
#endif

class	cgiResponse
{
	private:
	std::string	_status;
	std::string	_method;
	std::string	_contentType;
	int		_fd = 0;
	int		_reponseCode = 0;
	int		_failResponseCode = 0;
	size_t		_contentLength = 0;
	size_t		_bodyBegin = 0;
	std::map<std::string, std::string>	_headers = {};
	bool		_contentLengthSet = 0;
	bool		_done = 0;
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
	void	setFailResponseCode(int code) { _failResponseCode = code; };
	bool	isDone(){ return _done;}
	std::string	getMethod();
	std::string	getHeaders();
	std::vector<char>	getContent();
	class CgiRequestException : public std::exception
		{
			public:
				CgiRequestException(std::string message) : message(message) {};
				virtual const char* what() const throw();
			private:
				std::string message;
		};

	class ReadException : std::exception {
        const char * what() const noexcept { return ("Read failed"); }
    };
};

#endif
