/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 16:29:51 by hpatsi            #+#    #+#             */
/*   Updated: 2024/07/11 18:11:30 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <iostream>
# include <sstream>
# include <string>
# include <map>

# include <unistd.h>

# ifndef REQUEST_READ_BUFFER_SIZE
#  define REQUEST_READ_BUFFER_SIZE 1024
# endif

class HttpRequest
{
	public:
		HttpRequest( int socketFD );
		~HttpRequest( void );

		class ReadFailedException : public std::exception
		{
			public:
				const char* what() const throw() { return "read failed"; };
		};
	
	private:
		std::string	method;
		std::string	resource_path;
		std::map<std::string, std::string> paramenters;
};

#endif
