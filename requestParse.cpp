#include "requestParse.hpp"

Request::Request() {
	parseFunctions.push(&Request::parseBody);
	parseFunctions.push(&Request::parseFileds);
	parseFunctions.push(&Request::parseStartLine);

	parseFunctionsStarterLine.push(&Request::isProtocole);
	parseFunctionsStarterLine.push(&Request::isTarget);
	parseFunctionsStarterLine.push(&Request::isMethod);
}


void	Request::parseMessage(const char *buffer) {
	// char	buffer[BUFFER_SIZE+1] = {0};
	// int		byteRead;
	// if ((byteRead = recv(clientFd, buffer, BUFFER_SIZE, MSG_DONTWAIT)) < 0) {
	// 	perror("recv syscall failed");
	// 	exit(-1);
	// }

	remainingBuffer += buffer;
	stringstream	stream(remainingBuffer);
	while(!parseFunctions.empty()) {
		const auto& func = parseFunctions.top();
		if (!(this->*func)(stream))	return;
		parseFunctions.pop();
	}
	for (const auto& it: startLineComponents)	cerr << it << endl;
	for (const auto& it : headers)	cerr << it.first << ": " << it.second << endl;
	cerr << body << endl;
	exit(0);
}



void	Request::isProtocole(const string& http) const {
	if (http.size() == 8 && !strncmp(http.c_str(), "HTTP/", 5) && isdigit(http[5]) && http[6] == '.' && isdigit(http[7]))
		return ;
	throw("bad requst");
}

void	Request::isTarget(const string& str) const {
	const string	validCharachters = "-._~:/?#[]@!$&'()*+,;=";

	if (strncmp(str.c_str(), "http://", 7) && str[0] != '/')	throw("bad requst");
	for (const auto& c : str) {
		if (!iswalnum(c) && validCharachters.find(c) == string::npos)	throw("bad requst");
	}
}

void	Request::isMethod(const string& target) const {
	if (target == "GET" || target == "POST" || target == "DELETE") return ;
	throw("bad requst");
}

bool	Request::parseStartLine(stringstream& stream) {
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
	if (startLineComps.empty())
		return false;
	if (!lineEndedWithLF)	remainingBuffer = line;
	startLineCompsIt = startLineComps.begin();
	while(!parseFunctionsStarterLine.empty()) {
		const auto& func = parseFunctionsStarterLine.top();
		(this->*func)(*startLineCompsIt);
		startLineComponents.push_back(*startLineCompsIt);
		parseFunctionsStarterLine.pop();
		if (parseFunctionsStarterLine.empty())
			break;
		if (++startLineCompsIt == startLineComps.end())
			return false;
	}
	if (++startLineCompsIt != startLineComps.end() || !lineEndedWithLF)	throw("bad request");
	return true;
}



bool    Request::validFieldName(string& str) const {
	for (auto& c: str) {
		if (!iswalnum(c) && c != '_' && c != '-')	return false;
		c = tolower(c);
	}
	return true;
}

bool	Request::parseFileds(stringstream& stream) {
	string			line;
	static string	prvsFieldName;

	while(getline(stream, line) && line.size()) {
		string	fieldName;
		string	filedValue;

		if (!headers.empty() && (line[0] == ' ' || line[0] == '\t')) {
			headers[prvsFieldName] += " " + trim(line);
			continue ;
		}

		size_t colonIndex = line.find(':');
		fieldName = line.substr(0, colonIndex);
		if (colonIndex != string::npos && colonIndex+1 < line.size()) {
			filedValue = line.substr(colonIndex+1);
			filedValue = trim(filedValue);
		}
		if (colonIndex == string::npos || !validFieldName(fieldName)) {
			throw("bad request");
		}
		headers[fieldName] = filedValue;
		prvsFieldName = fieldName;
	}
	if (stream.eof()) {
		remainingBuffer = line;
		return false;
	}
	return true;
}


// void	Request::reconstructUri() {

// 	const string	scheme = "http://";
// 	string			authority = headers["host"];
// 	string			pathQuery = target;
// 	target = scheme + authority + target;
// }



bool	Request::parseBody(stringstream& stream) {
	static size_t	lenght;
	string	line;

	if (startLineComponents[0] != "POST") return true;
	if (headers.find("content-lenght") != headers.end() && lenght <= 0)
		lenght = stoi(headers["content-lenght"]) + 1;
	else if (headers.find("transfer-encoding") != headers.end() && headers["transfer-encoding"] == "chunked") {
		while (1) {
			if (lenght <= 0) {
				getline(stream, line);
				lenght = stoi(line) + 1; //in hex
				if (line == "0") { //read all body
					body += '\0';
					break ;
				}
			}
			if (line.size() == 0)	break ; //no  more content
			char *buff = new char[lenght];
			bzero(buff, lenght);
			stream.read(buff, lenght);
			lenght -= strlen(buff);
			body += buff;
			delete []buff;
			getline(stream, line); // consume the \n(its not included n the lenght)
		}
		return false;
	}
	else if (lenght <= 0)
		throw("unsoported tranfer-encoding");
	char *buff = new char[lenght];
	bzero(buff, lenght);
	stream.read(buff, lenght);
	lenght -= stream.gcount();
	body += buff;
	delete []buff;
	return true;
}

