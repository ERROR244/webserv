#include "httpSession.hpp"

inline string methodStringRepresentation(e_methods method) {
	switch (method)
	{
	case e_methods::GET:
		return "GET";
	case e_methods::POST:
		return "POST";
	case e_methods::DELETE:
		return "DELETE";
	case e_methods::NONE:
		return "unvalid";
	}
	return "unvalid";
}

inline void	validLocation(configuration& config, location** rules, const string& location) {
	if (config.locations.find(location) != config.locations.end())
		*rules = &config.locations.at(location);
}

inline void matchSubUriToConfigRules(configuration& config, location** rules, const bstring& bbuf, size_t start, size_t len) {
	string subUri = bbuf.substr(start, len).cppstring();
	validLocation(config, rules, subUri);
}

inline string	extractPath(configuration& config, location** rules, const bstring& bbuf, size_t start, size_t len) {
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

void	httpSession::Request::reconstructUri() {
	struct stat pathStat;
	string tmpOriginalPath;

	if (s.rules == NULL)
		throw(statusCodeException(404, "Not Found1"));
	if (s.rules->redirection) {
		s.statusCode = 301;
		s.codeMeaning = "Moved Permanently";
		s.returnedLocation = s.rules->reconfigurer;
		s.sstat = ss_sHeader;
		return ;
	} else {
		tmpOriginalPath = s.path;
		s.path.erase(s.path.begin(), s.path.begin()+s.rules->uri.size());
		s.path = s.rules->reconfigurer + s.path;
		if (s.path.find("/../") != string::npos || s.path.find("/..\0") != string::npos)
			throw(statusCodeException(403, "Forbidden"));
		isCGI();
		if (s.cgi) {
			cerr << "---CGIIIIIIIIIIIIIIII---" << endl;
			return;
		}
		if (stat(("." + s.path).c_str(), &pathStat) != 0) {
			throw statusCodeException(404, "Not Found"); // or something else
		}
		else if (S_ISDIR(pathStat.st_mode) && s.path.back() != '/') {
			s.statusCode = 301;
			s.codeMeaning = "Moved Permanently";
			s.returnedLocation = tmpOriginalPath + "/";
			s.sstat = ss_sHeader;
			return ;
		}
		cerr << s.path << endl;
		s.path = w_realpath(("." + s.path).c_str());
		cerr << s.path << endl;
	}
	if (find(s.rules->methods.begin(), s.rules->methods.end(), s.method) == s.rules->methods.end()) {
		cerr << "my m:  " << s.method << endl;
		for (const auto& m : s.rules->methods)
			cerr << m << endl;
		throw(statusCodeException(405, "Method Not Allowed"));
	}
	switch (s.method)
	{
	case GET: {
		stat(s.path.c_str(), &pathStat);
		if (S_ISDIR(pathStat.st_mode)) {
			string tmp = s.path;
			s.path += "/" + s.rules->index;
			cout << "to_check: " << s.path << endl;
			if (access(s.path.c_str(), F_OK) != -1)
				break;
			else {
				// false means off
				if (s.rules->autoIndex == false) {
					throw(statusCodeException(403, "Forbidden"));
				}
				else {
					string html = generate_autoindex_html(tmp, tmpOriginalPath);
					s.path = tmp + "/.index.html";
					if (html.empty()) {
						cout << "Failed to generate the HTML\n";
					} else {
						if (write_to_file(s.path, html) == true) {
							s.closeAutoIndex = true;
						}
					}
				}
			}
		}
		break;
	}
	case POST: {
		if (!s.rules->uploads.empty()) {
			if (stat(s.rules->uploads.c_str(), &pathStat) && !S_ISDIR(pathStat.st_mode))
				throw(statusCodeException(403, "Forbidden"));
		} else
			throw(statusCodeException(403, "Forbidden"));
		break;
	}
	case DELETE: {
		if (s.rules->uploads.empty())
			throw(statusCodeException(403, "Forbidden"));
		break;
	}
	default:
		throw(statusCodeException(400, "Bad Request1"));
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
						throw(statusCodeException(400, "Bad Request2"));
					s.method = GET;
					break;
				}
				case 4:{
					if (buffer.ncmp("POST", 4, i-len))
						throw(statusCodeException(400, "Bad Request3"));
					s.method = POST;
					break;
				}
				case 6:{
					if (buffer.ncmp("DELETE", 6, i-len))
						throw(statusCodeException(400, "Bad Request4"));
					s.method = DELETE;
					break;
				}
				default:
					throw(statusCodeException(400, "Bad Request5"));
				}
				s.sstat = ss_uri;
				len = 0;
				continue;
			}
			default: {
				if (len > 6)
					throw(statusCodeException(400, "Bad Request6"));
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
				if (buffer[i] != '/')
					throw(statusCodeException(400, "Bad Request7"));
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
				}
				continue;
			}
			case ' ': {
				if (s.path.empty())
					s.path = extractPath(s.config, &s.rules, buffer, i-len, len);
				else
					s.query = buffer.substr(i-len+1, len).cppstring();
				reconstructUri();
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
					throw(statusCodeException(400, "Bad Request8"));
			}
			++len;
			break;
		}
		case ss_httpversion: {
			switch (len)
			{
			case 7: {
				if (buffer.ncmp("HTTP/1.1", 8, i-len))
					throw(statusCodeException(400, "Bad Request9"));
				s.sstat = ss_starterlineNl;
				len = 0;
				continue;
			}
			case 8:
				throw(statusCodeException(400, "Bad Request10"));
			}
			++len;
			break;
		}
		case ss_starterlineNl: {
			switch (ch)
			{
			case '\r': {
				if (len != 0)
					throw(statusCodeException(400, "Bad Request11"));
				break;
			}
			case '\n': {
				s.sstat = ss_emptyline;
				cerr << "uri -> " << s.path << endl;
				return i+1;
			}
			default:
				throw(statusCodeException(400, "Bad Request12"));
			}
			++len;
			break;
		}
		default:
			break;
		}
	}
	throw(statusCodeException(400, "Bad Request13"));
	return(-1);
}
