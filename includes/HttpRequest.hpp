/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 16:29:51 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/11 16:46:54 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <iostream>
# include <string>
# include <map>

# include <unistd.h>

enum request_method
{
	GET,
	POST,
	DELETE
};

class HttpRequest
{
	public:
		HttpRequest( int socketFD );
		~HttpRequest( void );
	
	private:
		enum request_method	method;
		std::string	resource_path;
		std::map<std::string, std::string> paramenters;
};

#endif
