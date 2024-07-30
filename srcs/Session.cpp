#include "include/Session.hpp"
#include <chrono>
#include <random>
#include <sstream>

Session::Session()
{
	std::mt19937_64 rand64;
	std::stringstream stream;

	time_created = std::chrono::steady_clock::now();	
	stream << rand64;
	id = stream.str();
}

std::chrono::time_point<std::chrono::steady_clock> get_time_created();
{
	return time_created;
}
