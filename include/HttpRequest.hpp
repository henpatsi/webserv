/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 16:29:51 by hpatsi            #+#    #+#             */
/*   Updated: 2024/08/22 16:43:14 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <iostream>
# include <sstream>
# include <string>
# include <map>
# include <vector>
# include <algorithm>
# include <chrono>
# include <unistd.h>

# ifndef READ_BUFFER_SIZE
#  define READ_BUFFER_SIZE 1024
# endif

# ifndef MAX_URI_LENGTH
#  define MAX_URI_LENGTH 8192
# endif

# ifndef MAX_REQUEST_LINE_LENGTH // Includes room for 2 spaces and \r\n
#  define MAX_REQUEST_LINE_LENGTH (MAX_URI_LENGTH + std::string("DELETE").length() + std::string("HTTP/1.1").length() + 4)
# endif

# ifndef MAX_HEADER_SIZE
#  define MAX_HEADER_SIZE 4096
# endif

# ifndef SPACECHARS
#  define SPACECHARS " \f\n\r\t\v"
# endif

# ifndef DEBUG
#  define DEBUG 0
# endif

struct multipartData
{
	std::string					name;
	std::string					filename;
	std::string					contentType;
	std::vector<char>			data = {};
	std::string					boundary;
	std::vector<multipartData>	multipartDataVector = {};
};

int	extractMultipartData(std::vector<multipartData>& multipartDataVector, std::vector<char>& rawContent, std::string boundary);

template <typename T>
T safeNext(T i, T end, size_t amount);

class HttpRequest
{
	public:
		HttpRequest(void);
		HttpRequest(int connectionFD, long serverClientBodyLimit);

		// Reading content
		void	readRequest();

		// Getters
		std::string							getMethod(void) { return this->method; }
		std::string							getResourcePath(void) { return this->resourcePath; }
		std::string							getHttpVersion(void) { return this->httpVersion; }
		std::map<std::string, std::string>&	getHeaders(void) { return this->headers; }
		std::string							getHeader(std::string key) { return this->headers[key]; }
		std::string							getHost(void) { return this->host; }
		int									getPort(void) { return this->port; }
		std::vector<char>					getRawContent(size_t length = 0);
		std::vector<multipartData>			getMultipartData(void) { return this->multipartDataVector; }
		std::string							getQueryString(void) {return this->queryString; }
		int									getFailResponseCode(void) { return this->failResponseCode; }
		bool								isComplete(void) { return this->requestComplete; }
		bool								isCgi(void);
		std::string 						getFileExtension(void) { return this->fileExtension; }

		// Setters
		void	setFailResponseCode(int code);

		// Exceptions
		class RequestException : public std::exception
		{
			public:
				RequestException(std::string message) : message(message) {};
				virtual const char* what() const throw();
			private:
				std::string message;
		};
	
	private:
		// Initiation parameters
		int									requestFD;
		size_t								clientBodyLimit;
		std::vector<std::string>			allowedMethods = {"HEAD", "GET", "POST", "DELETE"};
		
		// Request state tracking
		size_t								requestLineLength = 0;
		size_t								headerLength = 0;
		size_t								contentLength = 0;
		unsigned int						totalRead = 0;
		int									failResponseCode = 0;
		bool								requestComplete = false;

		// Saved parameters
		std::vector<char>					rawRequest = {};
		std::string							method;
		std::string							resourcePath;
		std::string							queryString;
		std::string							httpVersion;
		std::map<std::string, std::string>	headers = {};
		std::string 						host;
		int									port = 80;
		std::vector<multipartData>			multipartDataVector = {};
		std::string							fileExtension;

		void	readFD(void);
		void	tryParseRequestLine(void);
		void	tryParseHeader(void);
		void	tryParseContent(void);

		void	extractURI(std::string URI);
		void	unchunkContent(std::vector<char>& chunkedVector);
		void	setErrorAndThrow(int code, std::string message);

		void	debugSummary(void);
};

#endif
