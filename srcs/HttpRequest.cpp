/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 16:29:53 by hpatsi            #+#    #+#             */
/*   Updated: 2024/08/09 08:39:28 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

// CONSTRUCTOR

HttpRequest::HttpRequest(){};

HttpRequest::HttpRequest(int connectionFD)
{
	this->requestFD = connectionFD;
}

void HttpRequest::readFD()
{
	char contentBuffer[READ_BUFFER_SIZE + 1] = {0};
	int bytesRead = 0;

	bytesRead = read(this->requestFD, contentBuffer, READ_BUFFER_SIZE);

	if (bytesRead == -1)
		setErrorAndThrow(500, "Read returned error");
	if (bytesRead == 0)
		setErrorAndThrow(400, "Nothing to read");

	totalRead = bytesRead;
	this->rawRequest.insert(this->rawRequest.end(), contentBuffer, contentBuffer + bytesRead);
}

// MEMBER FUNCTIONS

void HttpRequest::readRequest()
{
	try
	{
		readFD();

		if (this->requestLineLength == 0)
			tryParseRequestLine();
		if (this->requestLineLength > 0 && this->headerLength == 0)
			tryParseHeader();
		if (!this->requestComplete)
			tryParseContent();
		
		if (this->requestComplete)
			debugPrint();
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

void HttpRequest::tryParseRequestLine()
{
	std::string eol = "\r\n";
	std::vector<char>::iterator it = std::search(this->rawRequest.begin(), this->rawRequest.end(), eol.begin(), eol.end());
	if (it == this->rawRequest.end()) // First line not yet read
	{
		if (this->totalRead > MAX_REQUEST_LINE_LENGTH)
			setErrorAndThrow(400, "Request line too long");
		return ;
	}
	if (it == this->rawRequest.begin())
		setErrorAndThrow(400, "Request line is empty");

	std::string requestLineString(this->rawRequest.begin(), std::next(it,2)); // Get line including \n

	std::istringstream lineStream(requestLineString);
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
	queryString = URI.find('?') + 1;

	// Check HTTP version
	if (this->httpVersion.empty())
		setErrorAndThrow(400, "HTTP version is empty");
	if (this->httpVersion != "HTTP/1.1")
		setErrorAndThrow(505, "HTTP version not supported or not correctly formatted");

	this->requestLineLength = requestLineString.length();

	// std::cout << "Request line = '" << requestLineString << "'\n";
	// std::cout << "Request line length = " << this->requestLineLength << "\n";
}

void HttpRequest::tryParseHeader()
{
	std::string eoh = "\r\n\r\n";
	std::vector<char>::iterator headerStart = this->rawRequest.begin() + requestLineLength;
	std::vector<char>::iterator it = std::search(headerStart, this->rawRequest.end(), eoh.begin(), eoh.end());
	if (it == this->rawRequest.end()) // Header not yet fully read
		return ;

	std::string headerString(headerStart, std::next(it,4)); // Get string including end of header \r\n
	std::istringstream sstream(headerString);
	std::string line;

	// Transfers the headers into a map
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

	// Extract content info
	if (this->headers.find("transfer-encoding") != this->headers.end())
	{
		if (this->headers["transfer-encoding"] != "chunked")
			setErrorAndThrow(501, "Specified encoding not implemented");
	}
	else if (this->headers.find("content-length") != this->headers.end())
	{
		try
		{
			this->contentLength = std::stoi(this->headers["content-length"]);
		}
		catch (std::exception& e)
		{
			setErrorAndThrow(400, "Invalid content length");
		}
		if (this->contentLength > clientBodyLimit)
			setErrorAndThrow(413, "Request body larger than client body limit");
	}
	else
		this->requestComplete = true;

	this->headerLength = headerString.length();

	// std::cout << "Header line = '" << headerString << "'\n";
	// std::cout << "Header length = " << this->headerLength << "\n";
}

void HttpRequest::tryParseContent()
{
	if (this->headers.find("transfer-encoding") != this->headers.end() && this->headers["transfer-encoding"] == "chunked")
	{
		std::vector<char> rawContent = getRawContent();
		// TODO fix potential problem of a chunk containing 0\r\n\r\n, needs a loop to check for all chunks
		std::string eoc = "0\r\n\r\n";
		std::vector<char>::iterator it = std::search(rawContent.begin(), rawContent.end(), eoc.begin(), eoc.end());
		if (it == rawContent.end()) // Chunked content not fully read
			return ;
		unchunkContent(rawContent);
		it = std::next(this->rawRequest.begin(), this->requestLineLength + this->headerLength);
		this->rawRequest.erase(it, this->rawRequest.end());
		this->rawRequest.insert(this->rawRequest.end(), rawContent.begin(), rawContent.end());
	}
	else if (this->totalRead < this->requestLineLength + this->headerLength + this->contentLength) // Content length not fully read
		return ;
	else if (this->getHeader("content-type").find("multipart/form-data") != std::string::npos)
	{
		std::string boundary = this->getHeader("content-type");
		boundary = boundary.substr(boundary.find("boundary=") + 9);
		std::vector<char> rawContent = getRawContent();
		int extractRet = extractMultipartData(this->multipartDataVector, rawContent, boundary);
		if (extractRet != 0)
			setErrorAndThrow(extractRet, "Failed to extract multipart data");
	}

	// std::vector<char> rawContent = getRawContent();
	// std::string rawContentString = std::string(rawContent.begin(), rawContent.end());
	// std::cout << "Raw content = '" << rawContentString << "'\n";
	// std::cout << "Raw content length = " << rawContentString.length() << "\n";

	this->requestComplete = true;
}

void HttpRequest::unchunkContent(std::vector<char>& chunkedVector)
{
	std::vector<char> unchunkedVector;
	std::vector<char>::iterator start;
	std::vector<char>::iterator end;
	std::string eol = "\r\n";
	size_t chunkSize;

	start = chunkedVector.begin();

	while (true)
	{
		end = std::search(start, chunkedVector.end(), eol.begin(), eol.end());
		if (end == chunkedVector.end())
			setErrorAndThrow(400, "Invalid chunk format");
		if (end == chunkedVector.begin())
			setErrorAndThrow(400, "Missing chunk size");

		try
		{
			chunkSize = std::stoi(std::string(chunkedVector.begin(), std::prev(end, 1)));
		}
		catch(const std::exception& e)
		{
			setErrorAndThrow(400, "Invalid chunk size");
		}
		
		if (chunkSize == 0)
			break ;
		
		start = std::next(end, 2);
		end = std::next(start, chunkSize);
		if (end == chunkedVector.end())
			setErrorAndThrow(400, "Chunk size larger than remaining content");

		unchunkedVector.insert(unchunkedVector.end(), start, end);

		start = end;
		end = std::next(start, 2);
		if (std::search(start, end, eol.begin(), eol.end()) != start)
			setErrorAndThrow(400, "Chunk missing EOL");
		start = end;
	}

	chunkedVector = unchunkedVector;
}

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

	std::vector<char> rawContent = getRawContent();
	std::cout << "Raw content:\n  " << std::string(rawContent.begin(), rawContent.end()) << "\n";
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
