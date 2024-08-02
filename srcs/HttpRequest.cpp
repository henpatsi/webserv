/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 16:29:53 by hpatsi            #+#    #+#             */
/*   Updated: 2024/08/02 08:59:34 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

// CONSTRUCTOR

HttpRequest::HttpRequest(){};

HttpRequest::HttpRequest(int socketFD)
{
	try
	{
		std::string requestHeader = readRequestHeader(socketFD);
		std::istringstream sstream(requestHeader);

		parseFirstLine(sstream);
		parseHeader(sstream);
	}
	catch (RequestException& e)
	{
		std::cerr << "RequestException: " << e.what() << "\n";
	}
	catch(std::exception& e)
	{
		std::cerr << "RequestException: " << e.what() << "\n";
		this->failResponseCode = 500;
		this->requestComplete = true;
	}
}

void HttpRequest::tryReadContent(int socketFD)
{
	try
	{
		readBody(socketFD);

		if (this->requestComplete)
		{
			parseBody();
			debugPrint();
		}
	}
	catch (RequestException& e)
	{
		std::cerr << "RequestException: " << e.what() << "\n";
	}
	catch(std::exception& e)
	{
		std::cerr << "RequestException: " << e.what() << "\n";
		this->failResponseCode = 500;
		this->requestComplete = true;
	}
}

// MEMBER FUNCTIONS

void HttpRequest::setErrorAndThrow(int responseCode, std::string message)
{
	this->failResponseCode = responseCode;
	this->requestComplete = true;
	throw RequestException(message);
}

void HttpRequest::debugPrint()
{
	/* DEBUG PRINT */
	std::cout << "\nMethod: " << this->method << "\n";
	std::cout << "Resource path: " << this->resourcePath << "\n";
	std::cout << "HTTP version: " << this->httpVersion << "\n";
	std::cout << "URI parameters:\n";
	for (auto param : this->URIParameters)
		std::cout << "  " << param.first << " = " << param.second << "\n";
	std::cout << "Headers:\n";
	for (auto param : this->headers)
		std::cout << "  " << param.first << " = " << param.second << "\n";

	// std::cout << "Raw content:\n  " << std::string(this->rawContent.begin(), this->rawContent.end()) << "\n";
	if (this->multipartDataVector.size() > 0)
	{
		std::cout << "Multipart data:";
		for (multipartData data : this->multipartDataVector)
		{
			std::cout << "\n  Name: " << data.name << "\n  Filename: " << data.filename << "\n  content-type: " << data.contentType;
			// std::string dataString(data.data.begin(), data.data.end());
			// std::cout << "\n  Data: '" << dataString << "'\n";
			// if (data.filename == "")
			// {
			// 	std::cout << "\n  Nested multipart data:";
			// 	for (multipartData nestedData : data.multipartDataVector)
			// 	{
			// 		std::cout << "\n    Name: " << nestedData.name << "\n    Filename: " << nestedData.filename << "\n    content-type: " << nestedData.contentType;
			// 		// std::string nestedDataString(nestedData.data.begin(), nestedData.data.end());
			// 		// std::cout << "\n    Data: '" << nestedDataString << "'\n";
			// 	}
			// }
		}
	}
	else if (this->urlEncodedData.size() > 0)
	{
		std::cout << "Url encoded data:\n";
		for (auto param : this->urlEncodedData)
			std::cout << "  " << param.first << " = " << param.second << "\n";
	}

	std::cout << "\nREQUEST INFO FINISHED\n\n";
}

bool HttpRequest::readLine(int socketFD, std::string& line, int timeoutMilliseconds)
{
	// Reads 1 byte at a time until it reaches end of line to not read too much
	char readBuffer[2] = {0};
	int bytesRead = 0;
	int failedReads = 0;
	int failReadMax = timeoutMilliseconds / READ_ERROR_RETRY_MS;
	line = "";

	while (1)
	{
		bytesRead = read(socketFD, readBuffer, 1);
		if (bytesRead == -1) // Nothing to read
		{
			if (timeoutMilliseconds == 0) // If no timeout, fail immediately
				return false;
			failedReads += 1;
			if (failedReads > failReadMax) // After timeoutMilliseconds, throw error
				setErrorAndThrow(408, "Reading line timed out");
			std::this_thread::sleep_for(std::chrono::milliseconds(READ_ERROR_RETRY_MS));
			continue ;
		}
		failedReads = 0; // Reset timeout if read succeeds
		if (bytesRead == 0)
			setErrorAndThrow(400, "Reached end of file while reading line");
		line += readBuffer[0];
		if (readBuffer[0] == '\n')
			break ;
	}
	return true;
}

std::string HttpRequest::readRequestHeader(int socketFD)
{
	bool firstLine = true;
	std::string requestString;
	size_t requestLineSize = 0;
	
	while (requestString.size() - requestLineSize < MAX_HEADER_SIZE)
	{
		std::string line;
		readLine(socketFD, line, HEADER_READ_TIMEOUT_MILLISECONDS); // TODO check that any read line is not too long
		if (line == "\r\n" && !firstLine) // Allow first line to be empty
			break ;
		if (requestLineSize == 0 && line != "\r\n") // First real line is the request line, exclude from MAX_HEADER_SIZE
			requestLineSize = line.size();
		requestString += line;
		firstLine = false;
	}

	std::cout << "\nRequest Message:\n" << requestString << "\n";
	return requestString;
}

bool HttpRequest::readContent(int socketFD)
{
	char contentBuffer[CONTENT_READ_BUFFER_SIZE + 1] = {0};
	int contentReadSize;
	int bytesRead = 0;

	while (this->remainingContentLength > 0)
	{
		contentReadSize = this->remainingContentLength > CONTENT_READ_BUFFER_SIZE ? CONTENT_READ_BUFFER_SIZE : this->remainingContentLength;
		bytesRead = read(socketFD, contentBuffer, contentReadSize);
		if (bytesRead == -1)
		{
			// std::cout << "Failed to read content\n";
			return false;
		}
		contentBuffer[bytesRead] = '\0';
		this->rawContent.insert(this->rawContent.end(), contentBuffer, contentBuffer + bytesRead);
		this->remainingContentLength -= bytesRead;
	}
	std::cout << "Content fully read\n";
	return true;
}

bool HttpRequest::readChunkedContent(int socketFD)
{
	std::string line;

	if (this->remainingContentLength == 0) // Read chunk size on each pass through
	{
		if (!readLine(socketFD, line)) // TODO might cause issues if partially read (but might be 400 if chunk not fully written when read?)
			return false;
		this->remainingContentLength = std::stoi(line.substr(0, line.find("\r")), 0, 16);
	}

	while (this->remainingContentLength > 0)
	{
		readContentLength += this->remainingContentLength;
		if (readContentLength > clientBodyLimit)
			setErrorAndThrow(413, "Chunked request body larger than client body limit");
		if (!readContent(socketFD))
			return false;
		if (!readLine(socketFD, line)) // Reads the empty line  // TODO might cause issues if not / partially read (but might be 400 if chunk not fully written when read?)
			return false;
		if (!readLine(socketFD, line)) // Read next chunk size if ready  // TODO might cause issues if partially read (but might be 400 if chunk not fully written when read?)
			return false; 
		this->remainingContentLength = std::stoi(line.substr(0, line.find("\r")), 0, 16);
	}
	std::cout << "Chunked content fully read\n";
	return true;
}

void HttpRequest::readBody(int socketFD)
{
	// Prioritize chunked content over content length
	if (this->headers.find("transfer-encoding") != this->headers.end())
	{
		if (this->headers["transfer-encoding"] == "chunked") // Read chunked content
			this->requestComplete = readChunkedContent(socketFD);
		else
			setErrorAndThrow(415, "Unsupported transfer encoding");
		return ;
	}

	if (remainingContentLength == 0) // First pass setup of remaining content length
	{
		try
		{
			if (this->headers.find("content-length") != this->headers.end())
				remainingContentLength = std::stoi(this->headers["content-length"]);
		}
		catch (std::exception& e)
		{
			setErrorAndThrow(400, "Invalid content length");
		}
	}

	// Do not read content if it is above client body size limit
	if (remainingContentLength > clientBodyLimit)
		setErrorAndThrow(413, "Request body larger than client body limit");

	if (remainingContentLength > 0) // Read content with content length
		this->requestComplete = readContent(socketFD);
	else if (this->headers.find("content-type") != this->headers.end())
		setErrorAndThrow(411, "No content length specified");
	else // Nothing to read specified
		this->requestComplete = true;
}

void HttpRequest::parseFirstLine(std::istringstream& sstream)
{
	std::string line;

	getline(sstream, line);
	if (line.empty() || line == "\r")
		setErrorAndThrow(400, "Request line is empty");
	if (line.back() != '\r') // TODO remove these checks if we decide to accept just \n
		setErrorAndThrow(400, "Request line should end in \\r\\n");

	std::istringstream lineStream(line);
	std::string URI;
	lineStream >> this->method;
	lineStream >> URI;
	lineStream >> this->httpVersion;

	// Check that all present
	if (this->method.empty() || this->method == "\r"
		|| URI.empty() || URI == "\r"
		|| this->httpVersion.empty() || this->httpVersion == "\r")
		setErrorAndThrow(400, "Request line not complete");

	// Check method
	if (std::find(this->allowedMethods.begin(), this->allowedMethods.end(), this->method) == this->allowedMethods.end())
		setErrorAndThrow(501, "Method not implemented");

	// Check URI
	if (URI.length() > MAX_URI_LENGTH)
		setErrorAndThrow(414, "URI too long");
	// Extracts path from the URI
	this->resourcePath = URI.substr(0, URI.find('?'));
	if (this->resourcePath.find("#") != std::string::npos) // # marks end of resource path
		this->resourcePath.erase(this->resourcePath.find("#"));
	if (this->resourcePath.empty())
		setErrorAndThrow(400, "Resource path is empty");
	// Extracts the parameters from the URI into a map
	if (URI.find('?') != std::string::npos && URI.back() != '?')
		extractURIParameters(this->URIParameters, URI.substr(URI.find('?') + 1));

	// Check HTTP version
	if (this->httpVersion.empty())
		setErrorAndThrow(400, "HTTP version is empty");
	if (this->httpVersion != "HTTP/1.1")
		setErrorAndThrow(505, "HTTP version not supported or not correctly formatted");
}

void HttpRequest::parseHeader(std::istringstream& sstream)
{
	std::string line;
	// Reads the headers into a map
	while (std::getline(sstream, line))
	{
		if (line == "\r")
			break ;
		if (line.back() != '\r' // Line must end in \r\n
			|| line.find(':') == std::string::npos) // Line must contain a colon
			setErrorAndThrow(400, "Invalid header line format");

		std::string key = line.substr(0, line.find(':'));
		std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c){ return std::tolower(c); });
		if (key.empty())
			setErrorAndThrow(400, "Header key is empty");
		if (key.find_first_of(SPACECHARS) != std::string::npos)
			setErrorAndThrow(400, "Header key contains space character");

		std::string value = line.substr(line.find(':') + 1);
		size_t valueStart = value.find_first_not_of(SPACECHARS);
		size_t valueEnd = value.find_last_not_of(SPACECHARS);
		if (valueStart == std::string::npos)
			value = "";
		else
			value = value.substr(valueStart, valueEnd - valueStart + 1);

		this->headers[key] = value;
	}

	// Extract host and port
	if (this->headers.find("host") == this->headers.end())
		setErrorAndThrow(400, "Host header missing");
	this->host = this->headers["host"].substr(0, this->headers["host"].find(':'));
	if (this->host.empty())
		setErrorAndThrow(400, "Missing host name");
	if (this->headers["host"].find(':') != std::string::npos)
	{
		if (this->headers["host"].back() == ':')
			setErrorAndThrow(400, "Missing port number");
		try
		{
			this->port = std::stoi(this->headers["host"].substr(this->headers["host"].find(':') + 1));
		}
		catch (std::exception& e)
		{
			setErrorAndThrow(400, "Invalid port number");
		}
	}
	else
		this->port = 80;
}

void HttpRequest::parseBody(void)
{
	if (this->rawContent.empty())
		return ;

	// Extracts the data from the body content from known content types
	if (this->getHeader("content-type").find("multipart/form-data") != std::string::npos)
	{
		std::string boundary = this->getHeader("content-type");
		boundary = boundary.substr(boundary.find("boundary=") + 9);
		int extractRet = extractMultipartData(this->multipartDataVector, this->rawContent, boundary);
		if (extractRet != 0)
			setErrorAndThrow(extractRet, "Failed to extract multipart data");
	}
	else if (this->getHeader("content-type").find("application/x-www-form-urlencoded") != std::string::npos)
	{
		std::string rawContentString(this->rawContent.begin(), this->rawContent.end());
		extractURIParameters(this->urlEncodedData, rawContentString);
	}
}

// HELPER FUNCTIONS

void extractURIParameters(std::map<std::string, std::string>& parametersMap, std::string parametersString)
{
	while (1)
	{
		std::string parameter = parametersString.substr(0, parametersString.find('&'));
		if (parameter.find('=') != std::string::npos && parameter.front() != '=' && parameter.back() != '=')
		{
			std::string key = parameter.substr(0, parameter.find('='));
			std::string value = parameter.substr(parameter.find('=') + 1);
			parametersMap[key] = value;
		}
		if (parametersString.find('&') == std::string::npos || parametersString.find('&') == parametersString.size() - 1)
			break ;
		parametersString = parametersString.substr(parametersString.find('&') + 1);
	}
}

int extractMultipartData(std::vector<multipartData>& multipartDataVector, std::vector<char>& rawContent, std::string boundary)
{
	std::string boundaryStartString = "--" + boundary;
	std::string boundaryEndString = "--" + boundary + "--";
	std::string headerEndString = "\r\n\r\n";

	std::vector<char>::iterator start;
	std::vector<char>::iterator end;

	start = rawContent.begin();
	
	// Loop through each section of the multipart data
	while (!std::equal(boundaryEndString.begin(), boundaryEndString.end(), start))
	{
		multipartData data;

		// Check boundary start and skip over it
		if (!std::equal(boundaryStartString.begin(), boundaryStartString.end(), start))
			return (400);
		start += boundaryStartString.size() + 2;

		// Get header end
		end = std::search(start, rawContent.end(), headerEndString.begin(), headerEndString.end());
		if (end == rawContent.end())
			return (400);

		// Read relevant headers into multipartData
		std::string headerSection(start, end);
		std::istringstream sstream(headerSection);
		std::string line;
		while (getline(sstream, line))
		{
			std::string lowercaseLine;
			lowercaseLine = line;
			std::transform(lowercaseLine.begin(), lowercaseLine.end(), lowercaseLine.begin(), [](unsigned char c){ return std::tolower(c); });

			if (lowercaseLine.find("content-disposition:") != std::string::npos)
			{
				if (line.find("name=\"") != std::string::npos)
				{
					data.name = line.substr(line.find("name=\"") + 6);
					data.name.erase(data.name.find("\""));
				}
				if (line.find("filename=\"") != std::string::npos)
				{
					data.filename = line.substr(line.find("filename=\"") + 10);
					data.filename.erase(data.filename.find("\""));
				}
			}
			else if (lowercaseLine.find("content-type:") != std::string::npos)
			{
				data.contentType = line.substr(lowercaseLine.find("content-type:") + 14);
				// Get boundary of nested multipart data
				if (data.contentType.find("boundary=") != std::string::npos)
				{
					data.boundary = data.contentType.substr(data.contentType.find("boundary=") + 9);
					data.contentType.erase(data.contentType.find(";"));
				}
			}
		}

		// Get body iterators
		start = end + headerEndString.size();
		end = std::search(start, rawContent.end(), boundaryStartString.begin(), boundaryStartString.end());
		if (end == rawContent.end())
			return (400);

		// Copy body into multipartData
		data.data.insert(data.data.end(), start, end - 2);

		// Get nested multipart data
		if (!data.boundary.empty())
		{
			int extractRet = extractMultipartData(data.multipartDataVector, data.data, data.boundary);
			if (extractRet != 0)
				return (extractRet);
		}

		multipartDataVector.push_back(data);

		start = end;
	}
	return (0);
}

// EXCEPTIONS

const char* HttpRequest::RequestException::what() const throw()
{
	return this->message.c_str();
}
