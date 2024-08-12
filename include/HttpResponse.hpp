/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/15 11:02:19 by hpatsi            #+#    #+#             */
/*   Updated: 2024/08/11 14:34:47 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

# include "HttpRequest.hpp"
# include "ServerConfig.hpp"

# include <string>
# include <iostream>
# include <fstream>
# include <filesystem>
# include <map>
# include <ctime>
# include <iterator>

# include <unistd.h>
# include <sys/types.h>
# include <sys/wait.h>

# ifndef FILE_READ_SIZE
#  define FILE_READ_SIZE 1024
# endif

int		writeMultipartData(std::vector<multipartData> dataVector, std::string directory);
bool	multipartDataContainsFile(std::vector<multipartData> dataVector);

class HttpResponse
{
	public:
		HttpResponse(void);
		HttpResponse(HttpRequest& request, Route& route);

		std::string getPath(void) { return this->path; }
		std::string getContentType(void) { return this->contentType; };
		int getResponseCode(void) { return this->responseCode; }
		std::vector<char> getContent(void) { return this->content; }
		std::vector<char> getResponse(void) { return this->response; }

		class ResponseException : public std::exception
		{
			public:
				ResponseException(std::string message) : message(message) {}
				virtual const char* what() const throw();
			private:
				std::string message;
		};

	private:
		std::string path;
		Route& route;
		HttpRequest& request;
		std::string contentType;
		int responseCode = 500;
		std::vector<char> content;
		std::vector<char> response;
		bool directoryListingAllowed = true; // TODO get this from config file
		std::map<int, std::string> defaultErrorMessages = {
			{ 400, "Bad Request" },
			{ 403, "Forbidden" },
			{ 404, "Not Found" },
			{ 405, "Method Not Allowed" },
			{ 408, "Request Timeout" },
			{ 411, "Length Required" },
			{ 413, "Payload Too Large" },
			{ 414, "URI Too Long" },
			{ 415, "Unsupported Media Type" },
			{ 500, "Internal Server Error" },
			{ 501, "Not Implemented" },
			{ 505, "HTTP Version Not Supported" }
		};
		std::map<int, std::string> customErrorPages = {
			{ 404, "html/400/404.html" } // TODO get these from config file
		};

		void buildDefaultErrorContent(int code);
		void setError(int code);
		void setErrorAndThrow(int code, std::string message);
		void buildResponse(void);
		void buildDirectoryList(void);
		void prepareHeadResponse(void);
		void prepareGetResponse(void);
		void preparePostResponse(void);
		void prepareDeleteResponse(void);
};

#endif
