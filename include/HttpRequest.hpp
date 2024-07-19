/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 16:29:51 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/19 08:20:24 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <iostream>
# include <sstream>
# include <string>
# include <map>
# include <vector>

#include <unistd.h>

# ifndef REQUEST_READ_BUFFER_SIZE
#  define REQUEST_READ_BUFFER_SIZE 2 // This should be able to account for the full header?
# endif

# ifndef CONTENT_READ_BUFFER_SIZE
#  define CONTENT_READ_BUFFER_SIZE 1024
# endif

# ifndef MAX_HEADER_SIZE
#  define MAX_HEADER_SIZE 4096
# endif

struct multipartData
{
	std::string name;
	std::string filename;
	std::string contentType;
	std::string data;
};

void extractUrlParameters(std::map<std::string, std::string>& parametersMap, std::string parametersString);

class HttpRequest
{
	public:
		HttpRequest(int socketFD);

		std::string	getMethod(void) { return this->method; }
		std::string	getResourcePath(void) { return this->resourcePath; }
		std::string	getUrlParameter(std::string key) { return this->urlParameters[key]; }
		std::string	getHeader(std::string key) { return this->headers[key]; }
		std::string	getRawContent(void) { return this->rawContent; }
		std::vector<multipartData> getMultipartData(void) { return this->multipartDataVector; }
		int			getFailResponseCode(void) { return this->failResponseCode; }
	
	private:
		std::string	method;
		std::string	resourcePath;
		std::string	httpVersion;
		std::map<std::string, std::string> urlParameters = {};
		std::map<std::string, std::string> headers = {};
		std::string rawContent;
		std::vector<multipartData> multipartDataVector = {};
		int	failResponseCode = 0;

		std::string readRequestHeader(int socketFD);
};

#endif
