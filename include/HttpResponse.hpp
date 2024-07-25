/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/15 11:02:19 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/25 11:07:50 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

# include "HttpRequest.hpp"

# include <string>
# include <iostream>
# include <fstream>
# include <filesystem>
# include <map>

# include <unistd.h>
# include <sys/types.h>
# include <sys/wait.h>

// TODO get this from config file
# ifndef SITE_ROOT
#  define SITE_ROOT "www/"
# endif

// TODO get this from config file
# ifndef UPLOAD_DIR	
#  define UPLOAD_DIR "uploads/"
# endif

int writeMultipartData(std::vector<multipartData> dataVector, std::string directory);

class HttpResponse
{
	public:
		HttpResponse(HttpRequest& request);

		std::string getPath(void) { return this->path; }
		std::string getContentType(void) { return this->contentType; };
		int getResponseCode(void) { return this->responseCode; }
		std::string getContent(void) { return this->content; }
		std::string getResponse(void) { return this->response; }

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
		std::string contentType;
		int responseCode = 500;
		std::string content;
		std::string response;
		bool directoryListingAllowed = true; // TODO get this from config file
		std::map<int, std::string> defaultErrorMessages = {
			{ 400, "Bad Request" },
			{ 403, "Forbidden" },
			{ 404, "Not Found" },
			{ 405, "Method Not Allowed" },
			{ 408, "Request Timeout" },
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
		void buildPath(std::string requestPath);
		void buildDirectoryList(HttpRequest& request);
		void prepareGetResponse(HttpRequest& request);
		void preparePostResponse(HttpRequest& request);
		void preparePutResponse(HttpRequest& request);
		void prepareDeleteResponse(HttpRequest& request);
};

#endif
