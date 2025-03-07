#include "httpSession.hpp"

inline void	validLocation(configuration* config, location** rules, const string& location) {
	if (config->locations.find(location) != config->locations.end())
		*rules = (&config->locations.at(location));
}

inline void matchSubUriToConfigRules(configuration* config, location** rules, const bstring& bbuf, size_t start, size_t len) {
	string subUri = bbuf.substr(start, len).cppstring();
	validLocation(config, rules, subUri);
}

inline string	extractPath(configuration* config, location** rules, const bstring& bbuf, size_t start, size_t& len) {
	string path = bbuf.substr(start, len).cppstring();
	if (path[path.size()-1] != '/') {
		validLocation(config, rules, path + "/");
	} else {
		path.erase(path.end()-1);
	}
	len = 0;
	return path;
}

void	httpSession::Request::isCGI() {
	size_t		pos = 0;
	cgiInfo		cgiVars;
	bool		foundAMatch = false;

	while (1) {
		pos = s.path.find('/', pos);
		string subUri = s.path.substr(0, (pos != string::npos) ? pos+1 : pos);
		size_t	dotPos = subUri.rfind('.');
		string subUriExt;
		if (dotPos != string::npos) {
			subUriExt = subUri.substr(dotPos);
			if (s.rules->cgis.find(subUriExt) != s.rules->cgis.end() && !access(subUri.c_str() ,F_OK)) {
				cgiVars.scriptUri = w_realpath(subUri.c_str());
				size_t barPos = subUri.rfind('/');
				cgiVars.scriptName = subUri.substr(barPos+1);
				cgiVars.exec = s.rules->cgis[subUriExt];
				if (s.path.size() > subUri.size()+1)
					cgiVars.path = s.path.substr(pos);
				cgiVars.query = s.query;
				foundAMatch = true;
			}
		}
		if (pos++ == string::npos)
			break;
	}
	if (foundAMatch == true)
		s.cgi = new Cgi(cgiVars);
}

void	httpSession::Request::reconstructUri() {
	struct stat pathStat;

	if (s.rules == NULL)
		throw(statusCodeException(404, "Not Found"));
	if (find(s.rules->methods.begin(), s.rules->methods.end(), s.method) == s.rules->methods.end())
		throw(statusCodeException(405, "Method Not Allowed"));
	if (s.rules->redirection) {
		s.statusCode = 301;
		s.codeMeaning = "Moved Permanently";
		s.sstat = e_sstat::sHeader;
		return ;
	} else {
		s.path.erase(s.path.begin(), s.path.begin()+s.rules->uri.size()-1);
		s.path = s.rules->reconfigurer + s.path;
		s.path = w_realpath(("." + s.path).c_str());
	}
	isCGI();
	if (s.cgi) {
		cerr << "CGIIIIIIIIIIIIIIII" << endl;
		return;
	}
	switch (s.method)
	{
	case GET: {
		stat(s.path.c_str(), &pathStat);
		if (S_ISDIR(pathStat.st_mode)) {
			s.path += "/" + s.rules->index;
			if (stat(s.path.c_str(), &pathStat))
				throw(statusCodeException(404, "Not Found"));
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
		if (!s.rules->uploads.empty()) {
			s.path = s.rules->uploads + s.path;
			s.path = w_realpath(("." + s.path).c_str());
		} else
			throw(statusCodeException(403, "Forbidden"));
	}
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
		case e_sstat::method: {
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
				}
				}
				s.sstat = e_sstat::uri;
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
		case e_sstat::uri: {
			switch (len)
			{
			case URI_MAXSIZE:
				throw(statusCodeException(414, "URI Too Long"));
			case 0: {
				if (buffer[i] != '/')
					throw(statusCodeException(400, "Bad Request"));
			}
			default: {
				switch (ch)
				{
				case '/': {
					matchSubUriToConfigRules(s.config, &s.rules, buffer, i-len, len+1);
					break;
				}
				case '?': {
					if (s.path.empty())
						s.path = extractPath(s.config, &s.rules, buffer, i-len, len);
					continue;
				}
				case ' ': {
					if (s.path.empty())
						s.path = extractPath(s.config, &s.rules, buffer, i-len, len);
					else
						s.query = buffer.substr(i-len+1, len).cppstring();
					reconstructUri();
					s.sstat = e_sstat::httpversion;
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
			}
			}
			++len;
			break;
		}
		case e_sstat::httpversion: {
			switch (len)
			{
			case 7: {
				if (buffer.ncmp("HTTP/1.1", 8, i-len))
					throw(statusCodeException(400, "Bad Request"));
				s.sstat = e_sstat::starterlineNl;
				len = 0;
				continue;
			}
			case 8:
				throw(statusCodeException(400, "Bad Request"));
			}
			++len;
			break;
		}
		case e_sstat::starterlineNl: {
			switch (ch)
			{
			case '\r': {
				if (len != 0)
					throw(statusCodeException(400, "Bad Request"));
				break;
			}
			case '\n': {
				s.sstat = e_sstat::emptyline;
				cerr << "uri -> " << s.path << endl;
				return i+1;
			}
			default:
				throw(statusCodeException(400, "Bad Request"));
			}
			++len;
			break;
		}
		}
	}
	throw(statusCodeException(400, "Bad Request"));
	return(-1);
}
