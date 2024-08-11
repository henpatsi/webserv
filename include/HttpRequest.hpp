/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 16:29:51 by hpatsi            #+#    #+#             */
/*   Updated: 2024/08/11 11:24:14 by hpatsi           ###   ########.fr       */
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

# ifndef READ_BUFFER_SIZE
#  define READ_BUFFER_SIZE 1024
# endif

# ifndef MAX_URI_LENGTH
#  define MAX_URI_LENGTH 8192
# endif

# ifndef MAX_REQUEST_LINE_LENGTH
#  define MAX_REQUEST_LINE_LENGTH 16384 // TODO make this a more reasonable length
# endif

# ifndef MAX_HEADER_SIZE
#  define MAX_HEADER_SIZE 4096
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
		HttpRequest(int connectionFD);

		// Getters
		std::string							getMethod(void) { return this->method; }
		std::string							getResourcePath(void) { return this->resourcePath; }
		std::string							getHttpVersion(void) { return this->httpVersion; }
		std::map<std::string, std::string>	getURIParameters(void) { return this->URIParameters; }
		std::string							getURIParameter(std::string key) { return this->URIParameters[key]; }
		std::map<std::string, std::string>	getHeaders(void) { return this->headers; }
		std::string							getHeader(std::string key) { return this->headers[key]; }
		std::string							getHost(void) { return this->host; }
		int									getPort(void) { return this->port; }
		std::vector<char>					getRawContent(void)
											{	
												std::vector<char> rawContent(std::next(this->rawRequest.begin(), this->requestLineLength + this->headerLength), this->rawRequest.end());
												return rawContent;
											}

		std::vector<multipartData>			getMultipartData(void) { return this->multipartDataVector; }
		std::map<std::string, std::string>	getUrlEncodedData(void) { return this->urlEncodedData; }
		std::string	getQueryString(void) {return this->queryString; }
		int									getFailResponseCode(void) { return this->failResponseCode; }

		// Reading content
		void								readRequest();
		bool								isComplete(void) { return this->requestComplete; }

		// FOR DEBUG
		size_t								getRemainingContentLength(void) { return this->remainingContentLength; }
		size_t								getReadContentLength(void) { return this->readContentLength; }

		// If fail occurs outside request parsing
		void								setFailResponseCode(int code) { this->failResponseCode = code; this->requestComplete = true; }

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
		std::string	queryString;
		std::map<std::string, std::string>	URIParameters = {};
		std::map<std::string, std::string>	headers = {};
		std::string 						host;
		int									port = 0;
		size_t								remainingContentLength = 0;
		size_t								readContentLength = 0; // To check that chunked does not go over max size
		// std::vector<char>					rawContent = {};
		std::vector<multipartData>			multipartDataVector = {};
		std::map<std::string, std::string>	urlEncodedData = {};
		int									failResponseCode = 0;
		bool								requestComplete = false;
		std::vector<std::string>			allowedMethods = {"HEAD", "GET", "POST", "DELETE"};


		


		int									requestFD;
		std::vector<char>					rawRequest = {};

		size_t								requestLineLength = 0;
		size_t								headerLength = 0;
		size_t								contentLength = 0;
		unsigned int						totalRead = 0;


		void	readFD(void);
		void	tryParseRequestLine(void);
		void	tryParseHeader(void);
		void	tryParseContent(void);

		void 	unchunkContent(std::vector<char>& chunkedVector);
		void	setErrorAndThrow(int code, std::string message);
		void	debugPrint(void);
};

#endif
