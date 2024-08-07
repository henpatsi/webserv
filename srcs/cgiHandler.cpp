#include "../include/cgiHandler.hpp"

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

void	cgiHandler::create_envs(char **envs, HttpRequest &request, ServerConfig &config, sockaddr_in &client_address)
{
	gateway << "GATEWAY_INTERFACE=CGI/1.1";
	envs[0] = (char *) gateway.str().c_str();
	path_info = request.getResourcePath();
	path_info = "PATH_INFO=" + path_info.substr(0, path_info.find_last_of("/") + 1);
	envs[1] = (char *) path_info.c_str();
	query_string = "QUERY_STRING=" + request.getQueryString();
	envs[2] = (char *) query_string.c_str();
	method = "REQUEST_METHOD=" + request.getMethod();
	envs[3] = (char *) method.c_str();
	script_name = "SCRIPT_NAME=" + request.getResourcePath();	
	envs[4] = (char *) script_name.c_str();
	server_protocol = "SERVER_PROTOCOL=HTTP/1.1";	
	envs[5] = (char *) server_protocol.c_str();
	content_length << "CONTENT_LENGTH=" << request.getRawContent().size();
	envs[6] = (char *) content_length.str().c_str();
	content_type = "CONTENT_TYPE=" + request.getHeaders().find("content-type")->second;
	envs[7] = (char *) content_type.c_str();
	remote_address = "REMOTE_ADDR=" + ntoa(client_address);
	envs[8] = (char *) remote_address.c_str();
	hostname = "SERVER_NAME=" + config.getName();
	envs[9] = (char *) hostname.c_str();
	port = "SERVER_PORT=" + std::to_string(config.getPorts().at(0));
	envs[10] = (char *) port.c_str();
	envs[11] = 0;
	//need client network address, server port and server name from somewhere??
}

int	cgiHandler::runCGI(HttpRequest &request, ServerConfig &config, sockaddr_in &client_address)
{
	char* envs[11];
	char* args[2];
	std::string cgi;
	int	toCGI[2];
	int	fromCGI[2];
	int	pid;

	args[0] = (char *)request.getResourcePath().c_str();
	args[1] = 0;
	create_envs(envs, request, config, client_address);
	if (pipe(toCGI) == -1 || pipe(fromCGI) == -1)
		throw PipeException();
	pid = fork();
	if (pid == 0)
	{
		if (dup2(toCGI[0], 0) == -1 || dup2(fromCGI[1], 1) == -1)
			throw DupException();
		if (close(toCGI[1]) == -1 || close(fromCGI[0] == -1)) 
			throw CloseException();
		execve(cgi.c_str(), (char **)args, (char **)envs);
	}
	write(toCGI[1], request.getRawContent().data(), request.getRawContent().size());
	if (close(toCGI[1] == -1 || close(fromCGI[1] == -1) || close(toCGI[0]) == -1))
		throw CloseException();
	return (fromCGI[0]);
}