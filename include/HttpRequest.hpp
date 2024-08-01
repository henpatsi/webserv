/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 16:29:51 by hpatsi            #+#    #+#             */
/*   Updated: 2024/08/01 14:55:01 by hpatsi           ###   ########.fr       */
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
# include <thread>

#include <unistd.h>

# ifndef CONTENT_READ_BUFFER_SIZE
#  define CONTENT_READ_BUFFER_SIZE 1024
# endif

# ifndef MAX_URI_LENGTH
#  define MAX_URI_LENGTH 8192
# endif

# ifndef MAX_HEADER_SIZE
#  define MAX_HEADER_SIZE 4096
# endif

# ifndef READ_ERROR_RETRY_MS
#  define READ_ERROR_RETRY_MS 1
# endif

# ifndef HEADER_READ_TIMEOUT_MILLISECONDS
#  define HEADER_READ_TIMEOUT_MILLISECONDS 100
# endif

# ifndef SPACECHARS
#  define SPACECHARS " \f\n\r\t\v"
# endif

// Temporary hard coded server config values

# ifndef clientBodyLimit
#  define clientBodyLimit 300000
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

void	extractURIParameters(std::map<std::string, std::string>& parametersMap, std::string parametersString);
int		extractMultipartData(std::vector<multipartData>& multipartDataVector, std::vector<char>& rawContent, std::string boundary);

class HttpRequest
{
	public:
		HttpRequest(void);
		HttpRequest(int socketFD);

		// Getters
		std::string							getMethod(void) { return this->method; }
		std::string							getResourcePath(void) { return this->resourcePath; }
		std::string							getHttpVersion(void) { return this->httpVersion; }
		std::map<std::string, std::string>	getURIParameters(void) { return this->URIParameters; }
		std::string							getURIParameter(std::string key) { return this->URIParameters[key]; }
		std::map<std::string, std::string>	getHeaders(void) { return this->headers; }
		std::string							getHeader(std::string key) { return this->headers[key]; }
		std::vector<char>					getRawContent(void) { return this->rawContent; }
		std::vector<multipartData>			getMultipartData(void) { return this->multipartDataVector; }
		std::map<std::string, std::string>	getUrlEncodedData(void) { return this->urlEncodedData; }
		int									getFailResponseCode(void) { return this->failResponseCode; }

		// Reading content
		void								tryReadContent(int socketFD);
		bool								isComplete(void) { return this->requestComplete; }

		// FOR DEBUG
		size_t								getRemainingContentLength(void) { return this->remainingContentLength; }
		size_t								getReadContentLength(void) { return this->readContentLength; }

		// If fail occurs outside request parsing
		void								setFailResponseCode(int code) { this->failResponseCode = code; }

		class RequestException : public std::exception
		{
			public:
				RequestException(std::string message) : message(message) {};
				virtual const char* what() const throw();
			private:
				std::string message;
		};
	
	private:
		std::string							method;
		std::string							resourcePath;
		std::string							httpVersion;
		std::map<std::string, std::string>	URIParameters = {};
		std::map<std::string, std::string>	headers = {};
		size_t								remainingContentLength = 0;
		size_t								readContentLength = 0; // To check that chunked does not go over max size
		std::vector<char>					rawContent = {};
		std::vector<multipartData>			multipartDataVector = {};
		std::map<std::string, std::string>	urlEncodedData = {};
		int									failResponseCode = 0;
		bool								requestComplete = false;
		std::vector<std::string>			allowedMethods = {"HEAD", "GET", "POST", "PUT", "DELETE", "OPTIONS"};

		void		debugPrint(void);
		void		setErrorAndThrow(int code, std::string message);
		bool		readLine(int socketFD, std::string& line, int timeoutMilliseconds = 0);
		std::string	readRequestHeader(int socketFD);
		bool		readContent(int socketFD);
		bool		readChunkedContent(int socketFD);
		void		readBody(int socketFD);
		void		parseFirstLine(std::istringstream& sstream);
		void		parseHeader(std::istringstream& sstream);
		void		parseBody(void);
};

#endif
