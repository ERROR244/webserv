#include "requestParse.hpp"

Request::Request() : parseProgress(0), parser{&Request::parseStartLine, &Request::parseFileds, &Request::parseBody} {}


void	Request::parseMessage(const char *buffer) {
	if (readAllRequest == true) {
		cerr << method << endl;
		cerr << target << endl;
		cerr << httpVersion << endl;
		for (const auto& it : headers)	cerr << it.first << ": " << it.second << endl;
		cerr << body << endl;
		exit(0);
	}
	// char	buffer[BUFFER_SIZE+1];
	// int		byteRead;
	// if ((byteRead = recv(clientFd, buffer, BUFFER_SIZE, MSG_DONTWAIT)) < 0) {
	// 	perror("recv syscall failed");
	// 	exit(-1);
	// }

	remainingBuffer += buffer; // appendind the new data to remaining old one;
	stringstream	stream(remainingBuffer);
	(this->*parser[parseProgress])(stream);
}



void	Request::isProtocole(const string& httpVersion) const {
	//if the client is usin https reject the request
	if (!strncmp(httpVersion.c_str(), "HTTP/", 5) && isdigit(httpVersion[5]) && httpVersion[6] == '.' && isdigit(httpVersion[7]))
		return ;
	throw("bad requst");;
}

void	Request::isTarget(const string& str) const {
	const string	validCharachters = "-._~:/?#[]@!$&'()*+,;="; // valid charachters that can be in a target reques

	if (strncmp(str.c_str(), "http://", 7) && str[0] != '/')	throw("bad requst");; //if not origin form || absolute form
	for (const auto& c : str) {
		if (!iswalnum(c) && validCharachters.find(c) == string::npos)	throw("bad requst");;
	}
}

void	Request::isMethod(const string& target) const {
	if (target == "GET" || target == "POST" || target == "DELETE") return ;
	throw("bad requst");
}

void	Request::parseStartLine(stringstream& stream) {
	static int					startLineParseProgress;
	bool						lineEndedWithLF = false;
	string						line;
	vector<string>				startLineComps;
	vector<string>::iterator	startLineCompsIt;

	getline(stream, line);
	if (!stream.eof()) {
		line += '\n';
		lineEndedWithLF = true;
	}
	startLineComps = split_ws(line);
	if (startLineComps.empty()) return ;
	if (!lineEndedWithLF)	remainingBuffer = line; // adding the left overs if the line didn't end with LF
	startLineCompsIt = startLineComps.begin();
	switch (startLineParseProgress)
	{
	case 0: {
		isMethod(*startLineCompsIt);
		method = *startLineCompsIt;
		++startLineParseProgress;
		if (++startLineCompsIt == startLineComps.end())	break;
	}
	case 1: {
		isTarget(*startLineCompsIt);
		target = *startLineCompsIt;
		++startLineParseProgress;
		if (++startLineCompsIt == startLineComps.end())	break;
	}
	case 2: {
		isProtocole(*startLineCompsIt);
		httpVersion = *startLineCompsIt;
		++startLineParseProgress;
		break;
	}
	case 3:
		if (++startLineCompsIt != startLineComps.end())	throw("bad request");
	}
	if (lineEndedWithLF && startLineParseProgress != 3)
		throw("bad request");
	else if (startLineParseProgress == 3) {
		++parseProgress;
		parseFileds(stream);
	}
}



bool    Request::validFieldName(const string& str) const {
	for (const auto& c: str) {
		if (!iswalnum(c) && c != '_' && c != '-')	return false;
	}
	return true;
}

void	Request::parseFileds(stringstream& stream) {
	//if line starting with /t ot /sp that means its a continuation for a line foldin;
	string	line;
	string	prvsFieldName;

	while(getline(stream, line) && line.size()) {
		string	fieldName;
		string	filedValue;//can be empty

		if (!headers.empty() && (line[0] == ' ' || line[0] == '\t')) { //handle for line folding
			headers[prvsFieldName] += " " + trim(line);
			continue ;
		}

		size_t colonIndex = line.find(':');
		fieldName = line.substr(0, colonIndex); // convert to lower case
		if (colonIndex != string::npos && colonIndex+1 < line.size()) { //checking there's a value
			filedValue = line.substr(colonIndex+1);
			filedValue = trim(filedValue); //trimin any OWS
		}
		if (!validFieldName(fieldName)) { // a-z && 1-9 && -_
			throw("bad request");
		}
		headers[fieldName] = filedValue;
		prvsFieldName = fieldName;
	}
	if (stream.eof()) {
		remainingBuffer = line;
		return ;
	}
	++parseProgress;
	if (method == "POST")	parseBody(stream);
	else	readAllRequest = true;
}


void	Request::reconstructUri() {

	const string	scheme = "http://";
	string			authority = headers["host"];
	string			pathQuery = target;
	target = scheme + authority + target;
}



void	Request::parseBody(stringstream& stream) {
	static size_t	lenght;
	// static bool		startBodyParsin;
	string	line;
	// size_t	lenght;

	// if (!startBodyParsin) {
	// 	getline(stream, line);
	// 	if (stream.eof())
	// 		return ;
	// 	startBodyParsin = true;
	// }
	cerr << "here" << endl;
	if (headers.find("content-lenght") != headers.end() && lenght <= 0)
		lenght = stoi(headers["content-lenght"]) + 1;
	else if (headers.find("transfer-encoding") != headers.end() && headers["transfer-encoding"] == "chunked") {
		while (1) {
			if (lenght <= 0) {
				getline(stream, line);
				lenght = stoi(line); //in hex
				if (line == "0") { //read all body
					body += '\0';
					readAllRequest = true;
					break ;
				}
			}
			if (line.size() == 0)	break ; //no  more content
			char	buff[lenght+1] = {0};
			stream.read(buff, lenght);
			lenght -= strlen(buff);
			body += buff;
			getline(stream, line); // consume the \n(its not included n the lenght)
		}
		return ;
	}
	else if (lenght <= 0)
		throw("unsoported tranfer-encoding");
	char	buff[lenght+1] = {0};
	stream.read(buff, lenght);
	lenght -= stream.gcount();
	body += buff;
	if (lenght <= 0) readAllRequest = true;
}

