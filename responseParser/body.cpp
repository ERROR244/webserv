#include "requestParse.hpp"

int	Request::openTargetFile() const {
	int fd;

	if (fd = open(target.c_str(), O_WRONLY | O_CREAT, 0644) < 0) {
		perror("open failed: responseParser/body.cpp l: 7"); exit(-1);
	}
	return (fd);
}

bool	Request::contentLengthBased(stringstream& stream) {
	static int	length;
	static int	targetFileFD;

	if (!targetFileFD)
		targetFileFD = openTargetFile();
	if (!length) {
		try {
			length = stoi(headers["content-length"]);
		}
		catch(...) {
			perror("unvalid number in content length"); exit(-1);
		}
	}

	char buff[length+1] = {0};
	stream.read(buff, length);
	length -= stream.gcount();
	write(targetFileFD, buff, stream.gcount());
	return (length > 0) ? false : true;
}

bool	Request::transferEncodingChunkedBased(stringstream& stream) {
	static int	length;
	static int	targetFileFD;

	if (!targetFileFD)
		targetFileFD = openTargetFile();
	while (1) {
		string	line;
		if (length <= 0) {
			if (getline(stream, line)) {
				if (stream.eof()) {
					remainingBuffer = line;
					return false;
				}
				perror("getline failed"); exit(-1);
			}
			try {
				length = stoi(line);
			}
			catch(...) {
				perror("unvalid number in chunked length"); exit(-1);
			}
			if (line == "0")	return true;
		}
		char buff[length+1] = {0};
		stream.read(buff, length);
		if (stream.gcount() == 0) return false;
		length -= stream.gcount();
		write(targetFileFD, buff, stream.gcount());
		getline(stream, line); // consume the \n(its not included n the length)
	}
}



bool	Request::parseBody(stringstream& stream) {
	if (headers.find("content-length") != headers.end())
		parseFunctions.push(&Request::contentLengthBased);
	else if (headers.find("transfer-encoding") != headers.end() && headers["transfer-encoding"] == "chunked")
		parseFunctions.push(&Request::transferEncodingChunkedBased);
	else
		throw("unsoported tranfer-encoding");
	return true;
}