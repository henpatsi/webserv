/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 16:29:51 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/12 08:54:34 by hpatsi           ###   ########.fr       */
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
		~HttpRequest(void);

		std::string getMethod(void) { return this->method; }
		std::string getResourcePath(void) { return this->resource_path; }
		std::string getParameter(std::string key) { return this->paramenters[key]; }
	
	private:
		std::string	method;
		std::string	resource_path;
		std::map<std::string, std::string> paramenters = {};
};

#endif
