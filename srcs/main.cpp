/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/10 11:37:37 by hpatsi            #+#    #+#             */
/*   Updated: 2024/08/11 16:34:55 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerManager.hpp"

int main(int argc, char *argv[])
{
	std::string filePath;

	if (argc > 2)
	{
		std::cerr << "Usage: ./webserv <config file path>\n";
		return 1;
	}
	else if (argc == 2)
		filePath = argv[1];
	else
		filePath = "/etc/conf/1.conf";

	try
	{
		ServerManager s(filePath);
		s.runServers();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}
