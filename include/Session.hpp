#ifndef SESSION_HPP
# define SESSION_HPP

# include <string>
# include <chrono>

class Session {
private:
	std::string id;
	std::chrono::time_point<std::chrono::steady_clock>	time_created;
public:
	Session();
	bool is_valid();
};
#endif
