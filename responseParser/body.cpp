#include "requestParse.hpp"

bool	Request::parseBody(stringstream& stream) {
	//sending the respons here
	static int	length;
	string	line;

	if (method != "POST") return true;
	remainingBuffer.clear();
	if (headers.find("content-length") != headers.end() && length <= 0)
		length = stoi(headers["content-length"]);
	else if (headers.find("transfer-encoding") != headers.end() && headers["transfer-encoding"] == "chunked") {
		while (1) {
			if (length <= 0) {
				getline(stream, line);
				if (stream.eof()) {
					remainingBuffer = line;
					return false;
				}
				length = stoi(line);
				if (line == "0")	return true;
			}
			char *buff = new char[length+1];
			memset(buff, 0, length+1);
			stream.read(buff, length);
			if (stream.gcount() == 0) return false;
			length -= stream.gcount();
			body += buff;
			delete []buff;
			//write to the file here
			getline(stream, line); // consume the \n(its not included n the length)
		}
		return false;
	}
	else if (length <= 0)
		throw("unsoported tranfer-encoding");
	char *buff = new char[length+1];
	memset(buff, 0, length+1);
	stream.read(buff, length);
	length -= stream.gcount();
	body += buff;
	delete []buff;
	//write to the file here
	if (length > 0)	return false;
	return true;
}