#include "requestParse.hpp"

vector<string>	Request::splitStarterLine(string& str) {
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
		remainingStr += *(str.begin()+pos); //movin the string to point to what i ll add to the remaining buffer;
		++pos;
	}
	str = remainingStr;
	return strings;
}

void	Request::reconstructAndParseUri(string& uri) {
	if (uri[0] != '/') { //removing the scheme and authority and leaving just the path and query
		size_t pos = uri.find('/', 7);
		if (pos == string::npos) {
			uri = "/"; 
			targetPath = uri;
			return;
		}
		uri = uri.substr(pos);
	}
	size_t pos = uri.find('?');
	targetPath = uri.substr(0, pos);
	if (pos != string::npos)
		targetQuery = uri.substr(pos+1);
	//append the path of the files to the path (eg: /where -> /usr/nabil/Desktop/webserv/www/where)
	//file existence
	if (access(targetPath.c_str(), F_OK))
	{
		if (errno == ENOENT)
			throw("bad request:: file doesn't exist");
		else
			throw("access failed");
	}
	//cgi detection
	// isCGIScript(path);
}

void	Request::isProtocole(string& http) {
	if (http.size() == 8 && !strncmp(http.c_str(), "HTTP/", 5) && isdigit(http[5]) && http[6] == '.' && isdigit(http[7])) {
		this->httpVersion = http;
		return ;
	}
	throw("bad requst::protocole version");
}

void	Request::isTarget(string& target) {
	const string	validCharachters = "-._~:/?#[]@!$&'()*+,;=";

	if (strncmp(target.c_str(), "http://", 7) && target[0] != '/')	throw("bad requst:: unkown target form");
	for (const auto& c : target) {
		if (!iswalnum(c) && validCharachters.find(c) == string::npos)	throw("bad requst:: malformed target");
	}
	reconstructAndParseUri(target);
	this->target = target;
}

void	Request::isMethod(string& method) {
	if (method == "GET" || method == "POST" || method == "DELETE") {
		this->method = method;
		return ;
	}
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
	startLineComps = splitStarterLine(line);
	if (startLineComps.empty())
		return false;
	if (!lineEndedWithLF)	remainingBuffer = line;
	startLineCompsIt = startLineComps.begin();
	while(!parseFunctionsStarterLine.empty()) {
		const auto& func = parseFunctionsStarterLine.top();
		(this->*func)(*startLineCompsIt);
		parseFunctionsStarterLine.pop();
		if (parseFunctionsStarterLine.empty())
			break;
		if (++startLineCompsIt == startLineComps.end())
			return false;
	}
	if (++startLineCompsIt != startLineComps.end() || !lineEndedWithLF)	throw("bad request");
	//GET / HTTP/1.1 \n // test this;
	return true;
}