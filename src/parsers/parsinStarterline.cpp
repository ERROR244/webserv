#include "httpSession.hpp"

static inline string methodStringRepresentation(e_methods method) {
	switch (method)
	{
	case GET:
		return "GET";
	case POST:
		return "POST";
	case DELETE:
		return "DELETE";
	case NONE:
		return "unvalid";
	}
	return "unvalid";
}

static inline void	validLocation(configuration& config, location** rules, const string& location) {
	if (config.locations.find(location) != config.locations.end())
		*rules = &config.locations.at(location);
}

static inline void matchSubUriToConfigRules(configuration& config, location** rules, const bstring& bbuf, size_t start, size_t len) {
	string subUri = bbuf.substr(start, len).cppstring();
	validLocation(config, rules, subUri);
}

static inline string	extractPath(configuration& config, location** rules, const bstring& bbuf, size_t start, size_t len) {
	string path = bbuf.substr(start, len).cppstring();
	validLocation(config, rules, path);
	return path;
}

void	httpSession::Request::isCGI() {
	size_t		pos = 0;
	cgiInfo		cgiVars;
	bool		foundAMatch = false;

	while (1) {
		pos = s.path.find('/', pos);
		string subUri = s.path.substr(0, pos);
		size_t	dotPos = subUri.rfind('.');
		string subUriExt;
		if (dotPos != string::npos) {
			subUriExt = subUri.substr(dotPos);
			if (s.rules->cgis.find(subUriExt) != s.rules->cgis.end() && !access(("."+subUri).c_str() ,F_OK)) {
				cgiVars.scriptUri = w_realpath(("."+subUri).c_str());
				size_t barPos = subUri.rfind('/');
				cgiVars.scriptName = subUri.substr(barPos+1);
				cgiVars.exec = s.rules->cgis[subUriExt];
				if (s.path.size() > subUri.size()+1)
					cgiVars.path = s.path.substr(pos);
				cgiVars.query = s.query;
				cgiVars.method = methodStringRepresentation(s.method);
				foundAMatch = true;
			}
		}
		if (pos++ == string::npos)
			break;
	}
	if (foundAMatch == true) {
		s.cgi = new Cgi(cgiVars);
		s.path = cgiVars.scriptUri;
	}
}

bool	httpSession::Request::fileExistence() {
	struct stat pathStat;
	char lastCharInPath = s.path.size() ? s.path[s.path.size()-1] : 0;

	if (!lastCharInPath || stat(("." + s.path).c_str(), &pathStat))
		throw statusCodeException(404, "Not Found");
	else if (S_ISDIR(pathStat.st_mode) && lastCharInPath != '/' && s.method == GET) {
		s.statusCode = 303;
		s.codeMeaning = "See Other";
		s.returnedLocation = s.rawUri + "/";
		s.sstat = ss_sHeader;
		return false;
	} else if (S_ISDIR(pathStat.st_mode) && s.method == GET) {
		if (access(("." + s.path + s.rules->index).c_str(), F_OK) == -1 && s.rules->autoIndex == false)
			throw(statusCodeException(403, "Forbidden"));
		else if (access(("." + s.path + s.rules->index).c_str(), F_OK) == -1 && s.rules->autoIndex == true)
			s.showDirFiles = true;
		else
			s.path += s.rules->index;
	}
	s.path = w_realpath(("." + s.path).c_str());
	return true;
}

void	httpSession::Request::reconstructUri() {
	struct stat pathStat;

	if (s.rules->redirection) {
		s.statusCode = 301;
		s.codeMeaning = "Moved Permanently";
		s.returnedLocation = s.rules->reconfigurer;
		return ;
	} else {
		s.rawUri = s.path;
		s.path.erase(s.path.begin(), s.path.begin()+s.rules->uri.size());
		s.path = s.rules->reconfigurer + s.path;
		if (s.path.find("/../") != string::npos || s.path.find("/..\0") != string::npos)
			throw(statusCodeException(403, "Forbidden"));
		isCGI();
		if (s.cgi == NULL && fileExistence() == false)
			return;
	}
	if (find(s.rules->methods.begin(), s.rules->methods.end(), s.method) == s.rules->methods.end())
		throw(statusCodeException(405, "Method Not Allowed"));
	switch (s.method)
	{
	case GET: {
		s.sstat = ss_sHeader;
		break;
	}
	case POST: {
		if ((!s.rules->uploads.empty() && !stat(s.rules->uploads.c_str(), &pathStat) && S_ISDIR(pathStat.st_mode)) || s.cgi) {
			if (s.headers.find("content-length") == s.headers.end() && s.headers.find("transfer-encoding") == s.headers.end())
				throw(statusCodeException(400, "Bad Request"));
			s.sstat = ss_body;
			s.statusCode = 204;
        	s.codeMeaning = "No Content";
			break ;
		}
		throw(statusCodeException(403, "Forbidden"));
	}
	case DELETE: {
		if (s.cgi)
			throw(statusCodeException(501, "Not Implemented"));
		size_t pos;
		struct stat fileStat;

    	if (stat(s.path.c_str(), &fileStat))
    	    throw(statusCodeException(404, "Not Found"));
		if ((pos = s.path.find(s.rules->uploads)) == string::npos || pos) {
			throw(statusCodeException(403, "Forbidden"));
		}
		// else if (!(fileStat.st_mode & S_IWUSR))
		// 	throw(statusCodeException(403, "Forbidden"));
		s.sstat = ss_sHeader;
		s.statusCode = 204;
        s.codeMeaning = "No Content";
		break;
	}
	default:
		throw(statusCodeException(400, "Bad Request"));
	}
}

int	httpSession::Request::parseStarterLine(const bstring& buffer) {
	char			ch;
	size_t			len = 0;
	size_t			size = buffer.size();

	for (size_t i = 0; i < size; ++i) {
		ch = buffer[i];
		switch (s.sstat)
		{
		case ss_method: {
			switch (ch)
			{
			case ' ': {
				switch (len)
				{
				case 3: {
					if (buffer.ncmp("GET", 3, i-len))
						throw(statusCodeException(400, "Bad Request"));
					s.method = GET;
					break;
				}
				case 4:{
					if (buffer.ncmp("POST", 4, i-len))
						throw(statusCodeException(400, "Bad Request"));
					s.method = POST;
					break;
				}
				case 6:{
					if (buffer.ncmp("DELETE", 6, i-len))
						throw(statusCodeException(400, "Bad Request"));
					s.method = DELETE;
					break;
				}
				default:
					throw(statusCodeException(400, "Bad Request"));
				}
				s.sstat = ss_uri;
				len = 0;
				continue;
			}
			default: {
				if (len > 6)
					throw(statusCodeException(400, "Bad Request"));
			}
			}
			++len;
			break;
		}
		case ss_uri: {
			switch (len)
			{
			case URI_MAXSIZE:
				throw(statusCodeException(414, "URI Too Long"));
			case 0: {
				if (buffer[i] != '/' && s.path.empty())
					throw(statusCodeException(400, "Bad Request"));
			}
			}
			switch (ch)
			{
			case '/': {
				matchSubUriToConfigRules(s.config, &s.rules, buffer, i-len, len+1);
				break;
			}
			case '?': {
				if (s.path.empty()) {
					s.path = extractPath(s.config, &s.rules, buffer, i-len, len);
					len = 0;
					continue;
				}
				break;
			}
			case ' ': {
				if (s.path.empty())
					s.path = extractPath(s.config, &s.rules, buffer, i-len, len);
				else
					s.query = buffer.substr(i-len, len).cppstring();
				if (s.rules == NULL)
					throw(statusCodeException(404, "Not Found"));
				s.sstat = ss_httpversion;
				len = 0;
				continue;
			}
			case '-': case '.': case '_': case '~':
			case ':': case '#': case '[': case ']':
			case '@': case '!': case '$': case '&':
			case '\'': case '(': case ')': case '*':
			case '+': case ',': case ';': case '=':
				break;
			default:
				if (!iswalnum(ch))
					throw(statusCodeException(400, "Bad Request"));
			}
			++len;
			break;
		}
		case ss_httpversion: {
			switch (len)
			{
			case 7: {
				if (buffer.ncmp("HTTP/1.1", 8, i-len))
					throw(statusCodeException(400, "Bad Request"));
				s.sstat = ss_starterlineNl;
				len = 0;
				continue;
			}
			case 8:
				throw(statusCodeException(400, "Bad Request"));
			}
			++len;
			break;
		}
		case ss_starterlineNl: {
			switch (ch)
			{
			case '\r': {
				if (len != 0)
					throw(statusCodeException(400, "Bad Request"));
				break;
			}
			case '\n': {
				s.sstat = ss_emptyline;
				return i+1;
			}
			default:
				throw(statusCodeException(400, "Bad Request"));
			}
			++len;
			break;
		}
		default:
			break;
		}
	}
	throw(statusCodeException(400, "Bad Request"));
	return(-1);
}
