#include "requestParse.hpp"

Request::Request() {
	parseFunctions.push(&Request::parseBody);
	parseFunctions.push(&Request::parseFileds);
	parseFunctions.push(&Request::parseStartLine);

	parseFunctionsStarterLine.push(&Request::isProtocole);
	parseFunctionsStarterLine.push(&Request::isTarget);
	parseFunctionsStarterLine.push(&Request::isMethod);
}


void	Request::parseMessage(int clientFD) {
	char	buffer[BUFFER_SIZE+1] = {0};
	int		byteRead;
	if ((byteRead = recv(clientFD, buffer, BUFFER_SIZE, MSG_DONTWAIT)) < 0) {
		perror("recv syscall failed");
		exit(-1);
	}

	remainingBuffer += buffer;
	replace(remainingBuffer.begin(), remainingBuffer.end(), '\r', ' ');
	// cerr << "request buffer" << endl;
	// cerr << remainingBuffer << endl;
	stringstream	stream(remainingBuffer);
	while(!parseFunctions.empty()) {
		const auto& func = parseFunctions.top();
		if (!(this->*func)(stream))	return;
		parseFunctions.pop();
	}
	// for (const auto& it: startLineComponents)	cerr << it << endl;
	// for (const auto& it : headers)	cerr << it.first << ": " << it.second << endl;
	// cerr << body << endl;
	done = true;
}



void	Request::isProtocole(const string& http) const {
	if (http.size() == 8 && !strncmp(http.c_str(), "HTTP/", 5) && isdigit(http[5]) && http[6] == '.' && isdigit(http[7]))
		return ;
	throw("bad protocole");
}

void	Request::isTarget(const string& str) const {
	const string	validCharachters = "-._~:/?#[]@!$&'()*+,;=";

	if (strncmp(str.c_str(), "http://", 7) && str[0] != '/')	throw("bad target");
	for (const auto& c : str) {
		if (!iswalnum(c) && validCharachters.find(c) == string::npos)	throw("bad target");
	}
}

void	Request::isMethod(const string& target) const {
	if (target == "GET" || target == "POST" || target == "DELETE") return ;
	throw("bad methode");
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
	startLineComps = splitStartLine(line);
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

	while(getline(stream, line) && line.size() && line != " ") {
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
	static int	length;
	string	line;

	if (startLineComponents[0] != "POST") return true;
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
	if (length > 0)	return false;
	return true;
}

vector<string>	Request::splitStartLine(string& str) {
	const string	whiteSpace = " \t\n\r\f\v";
	string			remainingStr;
	int				pos = 0, i = 0;
	vector<string>	strings;

	while (str[i]) {
		if (whiteSpace.find(str[i]) != string::npos) {
			strings.push_back(str.substr(pos, i - pos));
			while (str[i] && whiteSpace.find(str[i]) != string::npos)	++i;
			pos = i;
		}
		else ++i;
	}
	while (str.begin()+pos != str.end()) {
		remainingStr += *(str.begin()+pos);
		++pos;
	}
	str = remainingStr;
	return strings;
}