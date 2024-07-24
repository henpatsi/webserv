/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 16:29:51 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/24 16:57:47 by hpatsi           ###   ########.fr       */
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

#include <unistd.h>

# ifndef CONTENT_READ_BUFFER_SIZE
#  define CONTENT_READ_BUFFER_SIZE 1024
# endif

# ifndef MAX_HEADER_SIZE
#  define MAX_HEADER_SIZE 4096
# endif

struct multipartData
{
	std::string					name;
	std::string					filename;
	std::string					contentType;
	std::vector<char>			data;
	std::string					boundary;
	std::vector<multipartData>	multipartDataVector = {};
};

void	extractURIParameters(std::map<std::string, std::string>& parametersMap, std::string parametersString);
int		extractMultipartData(std::vector<multipartData>& multipartDataVector, std::vector<char>& rawContent, std::string boundary);

class HttpRequest
{
	public:
		HttpRequest(int socketFD);

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

		class RequestException : public std::exception
		{
			public:
				virtual const char* what() const throw() { return "Error in request"; };
		};
	
	private:
		std::string							method;
		std::string							resourcePath;
		std::string							httpVersion;
		std::map<std::string, std::string>	URIParameters = {};
		std::map<std::string, std::string>	headers = {};
		std::vector<char>					rawContent = {};
		std::vector<multipartData>			multipartDataVector = {};
		std::map<std::string, std::string>	urlEncodedData = {};
		int									failResponseCode = 0;

		void		debugPrint(void);
		void		setErrorAndThrow(int code);
		std::string	readLine(int socketFD);
		std::string	readRequestHeader(int socketFD);
		void		readContent(int socketFD, int contentLength);
		void		readChunkedContent(int socketFD);
		void		parseFirstLine(std::istringstream& sstream);
		void		parseHeader(std::istringstream& sstream);
		void		parseBody(int socketFD);
};

#endif
