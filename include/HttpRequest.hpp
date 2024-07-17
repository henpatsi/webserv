/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 16:29:51 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/17 11:05:32 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <iostream>
# include <sstream>
# include <string>
# include <map>

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

class HttpRequest
{
	public:
		HttpRequest(int socketFD);

		std::string	getMethod(void) { return this->method; }
		std::string	getResourcePath(void) { return this->resourcePath; }
		std::string	getUrlParameter(std::string key) { return this->urlParameters[key]; }
		std::string	getHeader(std::string key) { return this->headers[key]; }
		std::string	getContent(void) { return this->content; }
		bool		isBadRequest(void) { return this->badRequest; }
	
	private:
		std::string	method;
		std::string	resourcePath;
		std::string	httpVersion;
		std::map<std::string, std::string> urlParameters = {};
		std::map<std::string, std::string> headers = {};
		std::string content;
		bool	badRequest = false;
};

#endif
