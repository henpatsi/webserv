/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/10 11:37:37 by hpatsi            #+#    #+#             */
/*   Updated: 2024/08/16 16:41:11 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv [request file]\n";
		return 1;
	}

	std::ifstream file(argv[1]);
	std::stringstream requestStringStream;
	requestStringStream << file.rdbuf();

	// std::cerr << "Request:\n'" << requestStringStream.str() << "'\n";

	Route route;
	route.acceptUpload = true;
	route.allowedMethods = ServerConfig::parseRequestMethod("HEAD") | ServerConfig::parseRequestMethod("GET") | ServerConfig::parseRequestMethod("POST") | ServerConfig::parseRequestMethod("DELETE");
	route.CGI = false;
	route.defaultAnswer = "";
	route.directoryListing = true;
	route.location = "";
	route.redirect = false;
	route.root = "./www";
	route.uploadDir = "./www/uploads";

	HttpRequest request(requestStringStream.str());
	HttpResponse response(request, route);

	std::vector<char> responseVector = response.getResponse();
	std::string responseString(responseVector.begin(), responseVector.end());
	std::cout << responseString;

	return response.getResponseCode();
}
