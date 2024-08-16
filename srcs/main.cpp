/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/10 11:37:37 by hpatsi            #+#    #+#             */
/*   Updated: 2024/08/16 13:41:54 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

bool debug = false;

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv [request]\n";
		return 1;
	}

	std::string requestString(argv[1]);

	// std::cerr << "Request:\n'" << requestString << "'\n";

	Route route;
	route.acceptUpload = false;
	route.allowedMethods = ServerConfig::parseRequestMethod("HEAD") | ServerConfig::parseRequestMethod("GET") | ServerConfig::parseRequestMethod("POST") | ServerConfig::parseRequestMethod("DELETE");
	route.CGI = false;
	route.defaultAnswer = "";
	route.directoryListing = true;
	route.location = "";
	route.redirect = false;
	route.root = "../www";
	route.uploadDir = "www/uploads";

	HttpRequest request(requestString);
	HttpResponse response(request, route);

	std::vector<char> responseVector = response.getResponse();
	std::string responseString(responseVector.begin(), responseVector.end());
	std::cout << responseString;

	return 0;
}
