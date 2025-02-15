#include "httpSession.hpp"

vector<string>	split(string& str) {
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

void	httpSession::Request::isCGI(location* loc) {
	size_t		pos = 0;
	cgiInfo		cgiVars;
	bool		foundAMatch = false;

	while (1) {
		pos = s.path.find('/', pos);
		string subUri = s.path.substr(0, pos);
		if (subUri == loc->alias || (subUri + '/') == loc->uri)
			subUri += '/' + loc->index;
		size_t	dotPos = subUri.rfind('.');
		string subUriExt = "";
		if (dotPos != string::npos)
			subUriExt = subUri.substr(dotPos);
		if (loc->cgi.find(subUriExt) != loc->cgi.end() && !access(("." + subUri).c_str() ,F_OK)) {
			cgiVars.scriptUri = w_realpath(("." + subUri).c_str());
			size_t barPos = subUri.rfind('/');
			cgiVars.scriptName = subUri.substr(barPos+1);
			cgiVars.exec = loc->cgi[subUriExt];
			if (s.path.size() > subUri.size()+1)
				cgiVars.path = s.path.substr(pos);
			cgiVars.query = s.query;
			foundAMatch = true;
		}
		if (pos++ == string::npos)
			break;
	}
	if (foundAMatch == true)
		s.cgi = new Cgi(cgiVars);
}

void	httpSession::Request::reconstructUri(location*	rules) {
	struct stat pathStat;

	if (find(rules->methods.begin(), rules->methods.end(), s.method) == rules->methods.end())
		throw(statusCodeException(405, "Method Not Allowed"));
	if (!rules->redirection.empty()) {
		s.statusCode = 301;
		s.codeMeaning = "Moved Permanently";
		//adding the location header to the response with the new path;
		return ;
	}
	if (!rules->alias.empty()) {
		s.path.erase(s.path.begin(), s.path.begin()+rules->uri.size() - 1);
		s.path = rules->alias + s.path;
	}
	isCGI(rules);
	if (s.cgi == NULL) {
		s.path = w_realpath(("." + s.path).c_str());
    	if (stat(s.path.c_str(), &pathStat))
			throw(statusCodeException(404, "Not Found"));
		if (S_ISDIR(pathStat.st_mode)) {//&& s->path == location
			s.path += "/" + rules->index;
			if (stat(s.path.c_str(), &pathStat))
				throw(statusCodeException(404, "Not Found"));
		}
	}
}

location*	httpSession::Request::getConfigFileRules() {
	size_t		pos = 0;
	location*	loc = NULL;

	if (s.config->locations.find(s.path) != s.config->locations.end()) {
		loc = &(s.config->locations.at(s.path));
		return loc;
	}
	while (1) {
		pos = s.path.find('/', pos);
		string subUri = s.path.substr(0, pos+1);
		if (s.config->locations.find(subUri) != s.config->locations.end())
			loc = &(s.config->locations.at(subUri));
		if (pos++ == string::npos)
			break;
	}
	return loc;
}

void	httpSession::Request::isProtocole(string& http) {
	if (http == "HTTP/1.1") {
		s.httpProtocole = http;
		return ;
	}
	else if (http.size() == 8 && !strncmp(http.c_str(), "HTTP/", 5) && isdigit(http[5]) && http[6] == '.' && isdigit(http[7]))
	throw(statusCodeException(505, "HTTP Version Not Supported"));
	throw(statusCodeException(400, "Bad Request"));
}

void	httpSession::Request::extractPathQuery(string& uri) {
	if (uri[0] != '/') {
		size_t pos = uri.find('/', 7);
		if (pos == string::npos)
			uri = "/";
		else
			uri = uri.substr(pos);
	}
	size_t pos = uri.find('?');
	s.path = uri.substr(0, pos);
	if (pos != string::npos)
		s.query = uri.substr(pos+1);
}

void	httpSession::Request::isTarget(string& target) {
	const string	validCharachters = "-._~:/?#[]@!$&'()*+,;=";

	if (strncmp(target.c_str(), "http://", 7) && target[0] != '/')
		throw(statusCodeException(400, "Bad Request"));
	for (const auto& c : target) {
		if (!iswalnum(c) && validCharachters.find(c) == string::npos)
			throw(statusCodeException(400, "Bad Request"));
	}
	extractPathQuery(target);
}

void	httpSession::Request::isMethod(string& method) {
	if (method == "GET" || method == "POST" || method == "DELETE")
		s.method = method;
	else
		throw(statusCodeException(400, "Bad Request"));
}


bool	httpSession::Request::parseStartLine(stringstream& stream) {
	string	line;
	location* rules;
	char absolutePath[1024];
	if (getline(stream, line)) {
		vector<string>	comps;
		comps = split(line);
		if (comps.size() != 3)
			throw(statusCodeException(400, "Bad Request"));
		isMethod(comps[0]);
		isTarget(comps[1]);
		isProtocole(comps[2]);
		if ((rules = getConfigFileRules()))
			reconstructUri(rules);
		else
			throw(statusCodeException(404, "Not Found"));
		return true;
	}
	remainingBuffer += line;
	return false;
}