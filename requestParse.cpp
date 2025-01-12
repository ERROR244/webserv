#include "requestParse.hpp"

Request::Request(string message) {
	parseMessage(message);
	if (target[0] == '/')
		reconstructUri();

	// //DEBUG
	// cerr << startLine << endl;
	// cerr << method << " " << target << " " << httpVersion << endl;
	// for (const auto& h : headers) {
	// 	cerr << h.first << ": " << h.second << endl;
	// }
	// cerr << body << endl;
	// //DEBUG
}



void	Request::parseMessage(const string& message) {
	string			line;
	stringstream	stream(message);

	while (getline(stream, line) && (line == "\r" || line.size() == 0)); //skiping any empty lines proceding the start-line
	parseStartLine(line);
	parseFileds(stream);
	if (method == "POST")	parseBody(stream);
}



bool	Request::isProtocole(const string& httpVersion) const {
	//if the client is usin https reject the request
	if (!strncmp(httpVersion.c_str(), "HTTP/", 5) && isdigit(httpVersion[5]) && httpVersion[6] == '.' && isdigit(httpVersion[7]))
		return true;
	return false;
}

bool	Request::isTarget(const string& str) const {
	const string	validCharachters = "-._~:/?#[]@!$&'()*+,;="; // valid charachters that can be in a target reques

	if (strncmp(str.c_str(), "http://", 7) && str[0] != '/')	return false; //if not origin form || absolute form
	for (const auto& c : str) {
		if (!iswalnum(c) && validCharachters.find(c) == string::npos)	return false;
	}
	return true;
}

bool	Request::isMethod(const string& target) const {
	if (target == "GET" || target == "POST" || target == "DELETE") return true;
	return false;
}

void	Request::parseStartLine(const string& startLine) {
	stringstream	stream(startLine);
	string			word;
	vector<string>	startLineComps;

	while (stream >> word) {
		startLineComps.push_back(word);
	}
	if (startLineComps.size() != 3 || !isProtocole(startLineComps[2]) || !isTarget(startLineComps[1]) || !isMethod(startLineComps[0])) {
		throw("bad request");
	}
	this->startLine = startLine;
	method = startLineComps[0];
	target = startLineComps[1];
	httpVersion = startLineComps[2];
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
	while(getline(stream, line) && line != "\r" && line.size() != 0) {
		string	fieldName;
		string	filedValue;//can be empty

		if (!headers.empty() && (line[0] == ' ' || line[0] == '\t')) { //handle for line folding
			headers[prvsFieldName] += " " + trim(line);
			continue ;
		}
		
		size_t colonIndex = line.find(':');
		fieldName = line.substr(0, colonIndex);
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
}



void	Request::parseBody(stringstream& stream) {
	// check for transer encoding
	string	line;
	//read byte by byte based on the content-lenght or chunked lenght
	while(getline(stream, line)) {
		body += line;
		body += "\n";
	}
}



void	Request::reconstructUri() {

	const string	scheme = "http://";
	string			authority = headers["host"];
	string			pathQuery = target;
	target = scheme + authority + target;
}
