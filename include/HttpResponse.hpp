/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/15 11:02:19 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/19 08:18:23 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

# include "HttpRequest.hpp"

# include <string>
# include <iostream>
# include <fstream>

# ifndef SITE_ROOT
#  define SITE_ROOT "./www/"
# endif

# ifndef UPLOAD_DIR	
#  define UPLOAD_DIR "uploads/"
# endif

class HttpResponse
{
	public:
		HttpResponse(HttpRequest& request);

		std::string getPath(void) { return this->path; }
		std::string getContentType(void) { return this->contentType; };
		int getResponseCode(void) { return this->responseCode; }
		std::string getContent(void) { return this->content; }
		std::string getResponse(void) { return this->response; }

	private:
		std::string path;
		std::string contentType;
		int responseCode;
		std::string content;
		std::string response;

		void setErrorValues(int responseCode, std::string message = "");
		void buildResponse(void);
		void buildPath(std::string requestPath);
		void prepareGetResponse(HttpRequest& request);
		void preparePostResponse(HttpRequest& request);
		void preparePutResponse(HttpRequest& request);
		void prepareDeleteResponse(HttpRequest& request);
};

#endif
