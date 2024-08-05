#include "../include/cgiHandler.hpp"


void	cgiHandler::create_envs(const char ** envs)
{
	this->gateway << "GATEWAY_INTERFACE=CGI/1.1";
	envs[0] = this->gateway.str().c_str();
	this->path_info = _request.getResourcePath();
	path_info = "PATH_INFO=" + path_info.substr(0, path_info.find_last_of("/") + 1);
	envs[1] = this->path_info.c_str();
	query_string = "QUERY_STRING=" + _request.getQueryString();
	envs[2] = this->query_string.c_str();
	method = "REQUEST_METHOD=" + _request.getMethod();
	envs[3] = method.c_str();
	script_name = "SCRIPT_NAME=" + _request.getResourcePath();	
	envs[4] = method.c_str();
	server_protocol = "SERVER_PROTOCOL=HTTP/1.1";	
	envs[5] = server_protocol.c_str();
	content_length << "CONTENT_LENGTH=" << _request.getRawContent().size();
	envs[6] = content_length.str().c_str();
	content_type = "CONTENT_TYPE=" + _request.getHeaders().find("content-type")->second;
	envs[7] = content_type.c_str();
	//need client network address, server port and server name from somewhere??
}

int	cgiHandler::runCGI()
{
	const char** envs = new const char*[16];
	char** args;
	int	toCGI[2];
	int	fromCGI[2];
	int	pid;
	std::string	tmp(_request.getRawContent().begin(), _request.getRawContent().end());

	*args = 0;
	create_envs(envs);
	if (pipe(toCGI) == -1 || pipe(fromCGI) == -1)
		throw PipeException();
	pid = fork();
	if (pid == 0)
	{
		if (dup2(toCGI[0], 0) == -1 || dup2(fromCGI[1], 1) == -1)
			throw DupException();
		if (close(toCGI[1]) == -1 || close(fromCGI[0] == -1)) 
			throw CloseException();
		execve(CGIcommand.c_str(), args, (char **)envs);
	}
	write(toCGI[1], tmp.c_str(), tmp.size());
	if (close(toCGI[1] == -1 || close(fromCGI[1] == -1) || close(toCGI[0]) == -1))
		throw CloseException();
	delete[] envs;
	return (fromCGI[0]);
}
		
	//set_envs
	//prepare pipes
	//fork
	//write message body into write end
	//execve
	//output cgi output fd
