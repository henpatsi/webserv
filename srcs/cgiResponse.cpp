#include "cgiResponse.hpp"

void cgiResponse::setErrorAndThrow(int responseCode, std::string message)
{
	_failResponseCode = responseCode;
	_done = true;
	throw RequestException(message);
}

cgiResponse::cgiResponse(void) {};

void	cgiResponse::setBodyBegin(std::string &temp)
{
	_bodyBegin = 0;
	size_t	found;
	found = temp.find_first_of("\r\n\r\n");
	if (found != temp.npos)
	{
		_bodyBegin = found + 4;
		return;
	}
	found = temp.find_first_of("\n\n");
	if (found != temp.npos)
		_bodyBegin = found + 2;
}

void	cgiResponse::checkMessageLength()
{
	if (_bodyBegin + 1 + _contentLength > _message.size())
		setErrorAndThrow(500, "CGI response content-length invalid");
}

void	cgiResponse::setContent()
{
	if (_contentLengthSet && _bodyBegin != 0)
	{
		_content.reserve(_contentLength);
		for (size_t i = 0; i < _contentLength && i + _bodyBegin < _message.size(); ++i)
		_content[i] = _message[i + _bodyBegin];
	}
	else if (_contentLengthSet == 0 && _bodyBegin != 0)
	{
		for (size_t i = 0; i + _bodyBegin < _message.size() - (_bodyBegin + 1); ++i)
		_content.push_back(_message[i + _bodyBegin]);
	}
}

void	cgiResponse::setHeader(std::string line)
{
	std::string key = line.substr(0, line.find(':'));
	std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c){ return std::tolower(c); });
	if (key.empty())
		setErrorAndThrow(500, "Header key is empty");
	if (key.find_first_of(SPACECHARS) != std::string::npos)
		setErrorAndThrow(500, "Header key contains space character");

	std::string value = line.substr(line.find(':') + 1);
	size_t valueStart = value.find_first_not_of(SPACECHARS);
	size_t valueEnd = value.find_last_not_of(SPACECHARS);
	if (valueStart == std::string::npos)
		value = "";
	else
		value = value.substr(valueStart, valueEnd - valueStart + 1);
	_headers[key] = value;
}

void	cgiResponse::parseBuffer()
{
	std::string	temp(_message.begin(), _message.end());
	std::stringstream s(temp);
	std::string	line;

	while (std::getline(s, line))
	{
		if (line.empty())
			break;
		if (line.back() == '\r')
			line.erase(line.end(line.npos - 1));
		if (line.empty())
			break;
		setHeader(line);
	}
	setBodyBegin(temp);
	checkMessageLength();
	checkRequiredHeaders();
	setContent();
	setFields();
}

bool	cgiResponse::readCgiResponse(int fd)
{
	int	bytesRead;

	bytesRead = read(fd, _buffer, 1024);
	if (bytesRead == -1)
		throw ReadException();
	else if (bytesRead == 0)
	{
		parseBuffer();
		_done = 1;
		return (_done);
	}
	else if (bytesRead > 0)
	{
		for (int i = 0; i < bytesRead; i++)
			_message.push_back(_buffer[i]);
		return (0);
	}
	return (0);
}
