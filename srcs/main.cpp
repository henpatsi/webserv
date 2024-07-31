#include "ServerConfig.hpp"
#include <iostream>
#include <string>
#include <iostream>
#include <fstream>

int	main(void)
{
	std::ifstream inputfile("confs/local.conf");
	std::ifstream printfile("confs/local.conf");
	std::stringstream s;
	std::string a;

	s << inputfile.rdbuf();
	std::cout << printfile.rdbuf();
	std::cout.flush();
	ServerConfig	config(s);
//	config.parseName("asdf:gsdf", "name:");
	std::cout << config.getName();
}
