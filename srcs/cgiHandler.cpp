#include "../include/cgiHandler.hpp"

static void setFdNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1)
        throw std::system_error();
    flags |= O_NONBLOCK;
    int res = fcntl(fd, F_SETFL, flags);
    if (res == -1)
        throw std::system_error();
}

std::string	ntoa(sockaddr_in &address)
{
	uint32_t a = address.sin_addr.s_addr;
	std::string s(std::to_string(a % 256));
	s.insert(0, ".");
	a /= 256;
	s.insert(0, std::to_string(a % 256));
	s.insert(0, ".");
	a /= 256;
	s.insert(0, std::to_string(a % 256));
	s.insert(0, ".");
	a /= 256;
	s.insert(0, std::to_string(a % 256));
	return (s);
}

char	**cgiHandler::create_envs(char **envs, HttpRequest &request, ServerConfig &config, sockaddr_in &client_address, Route& route)
{
	gateway = "GATEWAY_INTERFACE=CGI/1.1";
	envs[0] = (char *) gateway.c_str();
	path_info = "PATH_INFO=";
	path_info += route.root + route.location;
	envs[1] = (char *) path_info.c_str();
	query_string += "QUERY_STRING=" + request.getQueryString();
	envs[2] = (char *) query_string.c_str();
	method += "REQUEST_METHOD=" + request.getMethod();
	envs[3] = (char *) method.c_str();
	script_name += "SCRIPT_NAME=" + request.getResourcePath();	
	envs[4] = (char *) script_name.c_str();
	server_protocol += "SERVER_PROTOCOL=HTTP/1.1";	
	envs[5] = (char *) server_protocol.c_str();
	content_length += "CONTENT_LENGTH=" + std::to_string(request.getRawContent().size());
	envs[6] = (char *) content_length.c_str();
	content_type += "CONTENT_TYPE=" + request.getHeaders().find("content-type")->second;
	envs[7] = (char *) content_type.c_str();
	remote_address += "REMOTE_ADDR=" + ntoa(client_address);
	envs[8] = (char *) remote_address.c_str();
	hostname += "SERVER_NAME=" + config.getName();
	envs[9] = (char *) hostname.c_str();
	port += "SERVER_PORT=" + std::to_string(config.getPorts().at(0));
	envs[10] = (char *) port.c_str();
	envs[11] = 0;
	return envs;
	//need client network address, server port and server name from somewhere??
}

std::pair <int, int>	cgiHandler::runCGI(HttpRequest &request, ServerConfig &config, sockaddr_in &client_address, Route &route)
{
	char	*envs[16] = {};
	char* args[3] = {};
	int	toCGI[2];
	int	fromCGI[2];
	int	pid;
	
	if (route.CGI.find(request.getFileExtension()) == route.CGI.npos)
		throw RunCgiException("CGI not allowed");
	std::string cgiExecutable;
	if (request.getFileExtension() == "php")
		cgiExecutable = "php";
	if (request.getFileExtension() == "py")
		cgiExecutable = "python3";
	std::string cgiPath = route.root + request.getResourcePath();
	args[0] = (char *)cgiExecutable.c_str();
	args[1] = (char *)cgiPath.c_str();
	args[2] = 0;
	if (pipe(toCGI) == -1 || pipe(fromCGI) == -1)
		throw RunCgiException("Pipe failed");
	setFdNonBlocking(toCGI[0]);
	setFdNonBlocking(toCGI[1]);
	setFdNonBlocking(fromCGI[0]);
	setFdNonBlocking(fromCGI[1]);
	std::cout << "Executable: " << cgiExecutable << std::endl;
	std::cout << "Script :" << args[1] << std::endl;
	pid = fork();
	if (pid == 0)
	{
		chdir(cgiPath.substr(0, cgiPath.find_last_of("/")).c_str());
		if (dup2(toCGI[0], 0) == -1 || dup2(fromCGI[1], 1) == -1)
			throw RunCgiException("Dup failed");
		if (close(toCGI[1]) == -1 || close(toCGI[0]) == -1 || close(fromCGI[0]) == -1 || close(fromCGI[1]) == -1) 
			throw RunCgiException("Close failed");
		execve(cgiExecutable.c_str(), args, create_envs(envs, request, config, client_address, route));
		throw RunCgiException("Execve failed");
	}
	write(toCGI[1], request.getRawContent().data(), request.getRawContent().size());
	if (close(toCGI[1]) == -1 || close(fromCGI[1]) == -1 || close(toCGI[0]) == -1)
		throw RunCgiException("Close failed");
	std::pair <int, int> pair{pid, fromCGI[0]};
	return (pair);
}

const char* cgiHandler::RunCgiException::what() const throw()
{
	return this->message.c_str();
}
