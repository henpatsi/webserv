/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 16:29:51 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/13 15:09:33 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <iostream>

# include <sstream>
# include <string>
# include <map>

class HttpRequest
{
	public:
		HttpRequest(std::string requestMessageString);

		std::string getMethod(void) { return this->method; }
		std::string getResourcePath(void) { return this->resourcePath; }
		std::string getUrlParameter(std::string key) { return this->urlParameters[key]; }
		std::string getPostParameter(std::string key) { return this->postParameters[key]; }
	
	private:
		std::string	method;
		std::string	resourcePath;
		std::map<std::string, std::string> urlParameters = {};
		std::map<std::string, std::string> postParameters = {};
};

#endif
