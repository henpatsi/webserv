/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hpatsi <hpatsi@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/11 16:29:53 by hpatsi            #+#    #+#             */
/*   Updated: 2024/08/16 16:44:56 by hpatsi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

// CONSTRUCTOR

HttpRequest::HttpRequest(){};

HttpRequest::HttpRequest(int connectionFD)
{
	this->requestFD = connectionFD;
}

HttpRequest::HttpRequest(std::string requestString)
{
	try
	{
		this->rawRequest.insert(this->rawRequest.end(), requestString.begin(), requestString.end());
		this->totalRead = requestString.length();

		if (this->requestLineLength == 0)
			tryParseRequestLine();

		if (this->requestLineLength > 0)
			tryParseHeader();
		else
			setErrorAndThrow(400, "Incomplete request line");

		if (this->headerLength > 0)
			tryParseContent();
		else
			setErrorAndThrow(400, "Incomplete header");

		if (!this->requestComplete)
			setErrorAndThrow(400, "Incomplete content");
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


	// debugPrint();
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

	this->totalRead += bytesRead;
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
	std::vector<char>::iterator start = this->rawRequest.begin();
	std::vector<char>::iterator end = std::search(start, this->rawRequest.end(), eol.begin(), eol.end());
	bool emptyFirstLine = false;
	if (end == start) // Allow one empty line before request line
	{
		start = std::next(end, 2);
		end = std::search(start, this->rawRequest.end(), eol.begin(), eol.end());
		emptyFirstLine = true;
	}
	if (end == this->rawRequest.end()) // First line not yet read
	{
		if (this->totalRead > MAX_REQUEST_LINE_LENGTH)
			setErrorAndThrow(400, "Request line too long");
		return ;
	}
	if (end == start)
		setErrorAndThrow(400, "Request line is empty");

	std::string requestLineString(start, std::next(end, 2)); // Get line including \r\n
	if (requestLineString.length() > MAX_REQUEST_LINE_LENGTH)
		setErrorAndThrow(400, "Request line too long");

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
	// Check nothing extra is there
	std::string remaining;
	lineStream >> remaining;
	if (!remaining.empty())
		setErrorAndThrow(400, "Request line contains extra information");

	// Check method
	if (std::find(this->allowedMethods.begin(), this->allowedMethods.end(), this->method) == this->allowedMethods.end())
		setErrorAndThrow(501, "Method not implemented");
	// Extract all portions of the URI
	extractURI(URI);
	// Check HTTP version
	if (this->httpVersion.empty())
		setErrorAndThrow(400, "HTTP version is empty");
	if (this->httpVersion != "HTTP/1.1")
		setErrorAndThrow(505, "HTTP version not supported or not correctly formatted");

	this->requestLineLength += requestLineString.length();
	if (emptyFirstLine)
		this->requestLineLength += 2;

	// std::cerr << "- Request line parsed -\n";
	// std::cerr << "Request line = '" << requestLineString << "'\n";
	// std::cerr << "Request line length = " << this->requestLineLength << "\n";
}

void HttpRequest::tryParseHeader()
{
	std::string eoh = "\r\n\r\n";
	std::vector<char>::iterator headerStart = this->rawRequest.begin() + requestLineLength;
	std::vector<char>::iterator it = std::search(headerStart, this->rawRequest.end(), eoh.begin(), eoh.end());
	if (it == this->rawRequest.end()) // Header not yet fully read
	{
		if (this->totalRead - this->requestLineLength > MAX_HEADER_SIZE)
			setErrorAndThrow(400, "Header too long");
		return ;
	}

	std::string headerString(headerStart, std::next(it,4)); // Get string including end of header \r\n
	if (headerString.length() > MAX_REQUEST_LINE_LENGTH)
		setErrorAndThrow(400, "Header too long");
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
		if (this->headers.find(key) != this->headers.end())
			setErrorAndThrow(400, "Duplicate headers");

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
	else if (this->headers.find("content-type") != this->headers.end())
	{
		setErrorAndThrow(411, "Content length required");
	}
	else
		this->requestComplete = true;

	this->headerLength = headerString.length();

	// std::cerr << "- Header parsed -\n";
	// std::cerr << "Header line = '" << headerString << "'\n";
	// std::cerr << "Header length = " << this->headerLength << "\n";
}

void HttpRequest::tryParseContent()
{
	// std::cerr << "Content length = " << this->contentLength << "\n";
	// std::cerr << "Total read = " << this->totalRead << "\n";
	// std::cerr << "Content read = " << this->totalRead - this->requestLineLength - this->headerLength << "\n";

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
	else if (this->headers.find("content-type") != this->headers.end() && this->headers["content-type"].find("multipart/form-data") != std::string::npos)
	{
		std::string boundary = this->getHeader("content-type");
		boundary = boundary.substr(boundary.find("boundary=") + 9);
		std::vector<char> rawContent = getRawContent();
		int extractRet = extractMultipartData(this->multipartDataVector, rawContent, boundary);
		if (extractRet != 0)
			setErrorAndThrow(extractRet, "Failed to extract multipart data");
	}

	// std::cerr << "- Content parsed -\n";
	// std::vector<char> rawContent = getRawContent();
	// std::string rawContentString = std::string(rawContent.begin(), rawContent.end());
	// std::cerr << "Raw content = '" << rawContentString << "'\n";
	// std::cerr << "Raw content length = " << rawContentString.length() << "\n";

	this->requestComplete = true;
}

void HttpRequest::extractURI(std::string URI)
{
	if (URI.length() > MAX_URI_LENGTH)
		setErrorAndThrow(414, "URI too long");
	if (URI.find("#") != std::string::npos) // ignore anythign after #
		URI.erase(URI.find("#"));

	std::string fullPathString = URI.substr(0, URI.find('?'));
	fileExtension = fullPathString.substr(fullPathString.find_last_of("/"));
	fileExtension = fileExtension.substr(fileExtension.find_last_of(".") + 1);
	
	if (fullPathString.front() != '/') // TODO use commented out portion below if absolute path accepted
		setErrorAndThrow(400, "Invalid resource path");

	// // Extract host if any
	// if (fullPathString.front() != '/')
	// {
	// 	if (fullPathString.find("http://") != std::string::npos)
	// 	{
	// 		if (fullPathString.find("http://") != 0)
	// 			setErrorAndThrow(400, "Invalid URI format");
	// 		fullPathString.erase(0, 7);
	// 	}
	// 	if (!std::isdigit(fullPathString.front()))
	// 		this->resourcePathHost = fullPathString.substr(0, fullPathString.find('/'));
	// 	else
	// 		this->resourcePathIP = fullPathString.substr(0, fullPathString.find('/'));
	// 	fullPathString.erase(0, fullPathString.find('/'));
	// }

	// // Extract port if any
	// if (this->resourcePathHost.find(':') != std::string::npos)
	// {
	// 	try
	// 	{
	// 		this->resourcePathPort = std::stoi(this->resourcePathHost.substr(this->resourcePathHost.find(':') + 1));
	// 		this->resourcePathHost.erase(this->resourcePathHost.find(':'));
	// 	}
	// 	catch(const std::exception& e)
	// 	{
	// 		setErrorAndThrow(400, "Invalid port number in URI");
	// 	}
	// }

	// The resource path should be the remaining full path string
	this->resourcePath = fullPathString;
	if (this->resourcePath.empty())
		setErrorAndThrow(400, "Resource path is empty");

	// Extract the query string if any
	if (URI.find('?') != std::string::npos && URI.back() != '?')
		this->queryString = URI.substr(URI.find('?') + 1);

	// std::cerr << "URI host: '" << this->resourcePathHost << "'\n";
	// std::cerr << "URI port: '" << this->resourcePathPort << "'\n";
	// std::cerr << "URI IP: '" << this->resourcePathIP << "'\n";
}

void HttpRequest::unchunkContent(std::vector<char>& chunkedVector)
{
	std::string eol = "\r\n";
	std::vector<char> unchunkedVector;
	std::vector<char>::iterator start;
	std::vector<char>::iterator end;
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
			chunkSize = std::stoi(std::string(start, end), 0, 16);
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
	std::cerr << "\nMethod: " << this->method << "\n";
	std::cerr << "Resource path: " << this->resourcePath << "\n";
	std::cerr << "Query string: " << this->queryString << "\n"; 
	std::cerr << "HTTP version: " << this->httpVersion << "\n";
	std::cerr << "Headers:\n";
	for (auto param : this->headers)
		std::cerr << "  " << param.first << " = " << param.second << "\n";

	// std::vector<char> rawContent = getRawContent();
	// std::cerr << "Raw content:\n  " << std::string(rawContent.begin(), rawContent.end()) << "\n";
	if (this->multipartDataVector.size() > 0)
	{
		std::cerr << "Multipart data:";
		for (multipartData data : this->multipartDataVector)
		{
			std::cerr << "\n  Name: " << data.name << "\n  Filename: " << data.filename << "\n  content-type: " << data.contentType;
			// std::string dataString(data.data.begin(), data.data.end());
			// std::cerr << "\n  Data: '" << dataString << "'\n";
			// if (data.filename == "")
			// {
			// 	std::cerr << "\n  Nested multipart data:";
			// 	for (multipartData nestedData : data.multipartDataVector)
			// 	{
			// 		std::cerr << "\n    Name: " << nestedData.name << "\n    Filename: " << nestedData.filename << "\n    content-type: " << nestedData.contentType;
			// 		// std::string nestedDataString(nestedData.data.begin(), nestedData.data.end());
			// 		// std::cerr << "\n    Data: '" << nestedDataString << "'\n";
			// 	}
			// }
		}
	}

	std::cerr << "\nREQUEST INFO FINISHED\n\n";
}

std::vector<char>	HttpRequest::getRawContent(void)
{
	std::vector<char> rawContent(std::next(this->rawRequest.begin(), this->requestLineLength + this->headerLength), this->rawRequest.end());
	return rawContent;
}

// HELPER FUNCTIONS

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

bool	HttpRequest::isCgi(void)
{
	if (fileExtension == "php" || fileExtension == "py")
		return true;
	return false;
}

// EXCEPTIONS

const char* HttpRequest::RequestException::what() const throw()
{
	return this->message.c_str();
}
