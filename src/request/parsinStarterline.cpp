#include "httpSession.hpp"

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

	s.codeMeaning = "";
	if (find(s.rules->methods.begin(), s.rules->methods.end(), s.method) == s.rules->methods.end())
		throw(statusCodeException(405, "Method Not Allowed"));
	switch (s.method)
	{
		case GET: {
			if (s.rules->isRed) {
				s.statusCode = 301;
				s.codeMeaning = "Moved Permanently";
				//adding the location header to the response with the new path;
				s.codeMeaning = s.rules->aliasRed;
				return ;
			} else {
				s.path.erase(s.path.begin(), s.path.begin()+s.rules->url.size()-1);
				s.path = s.rules->aliasRed + s.path;
			}
			s.path = w_realpath(("." + s.path).c_str());
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
				s.path = s.rules->uploads;
				s.path.erase(s.path.end()-1);
				s.path = w_realpath(("." + s.path).c_str());
				if (stat(s.path.c_str(), &pathStat) && !S_ISDIR(pathStat.st_mode))
					throw(statusCodeException(403, "Forbidden"));
			} else
				throw(statusCodeException(403, "Forbidden"));
			break;
		}
		case DELETE: {
			if (!s.rules->uploads.empty()) {
				// s.path = s.rules->uploads + s.path;
				s.path = w_realpath(("." + s.path).c_str());
			} else
				throw(statusCodeException(403, "Forbidden"));
		}
	}
	isCGI();
}

int	httpSession::Request::parseStarterLine(const bstring& buffer) {
	char			ch;
	size_t			size = buffer.size();
	size_t			len = 0;

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
			}
			switch (ch)
			{
			case '/': {
				string subUri = buffer.substr(i-len, len+1).cppstring();
				if (s.config->locations.empty())
					cerr << "`" + subUri + "`" << endl;
				for (const auto& it : s.config->locations)
					cerr << it.first << " | " << it.second.url << endl;
				if (s.config->locations.find(subUri) != s.config->locations.end()) {
					cout << "YEAGH\n";
					s.rules = &(s.config->locations.at(subUri));
				}
				break;
			}
			case '?': {
				if (s.path.empty()) {
					s.path = buffer.substr(i-len, len).cppstring();
					if (s.path[s.path.size()-1] != '/') {
						s.path += '/';
						if (s.config->locations.find(s.path) != s.config->locations.end())
							s.rules = &(s.config->locations.at(s.path));
					}
					len = 0;
					continue;
				}
				break;
			}
			case ' ': {
				if (s.path.empty()) {
					s.path = buffer.substr(i-len, len).cppstring();
					if (s.path[s.path.size()-1] != '/') {
						s.path += '/';
						if (s.config->locations.find(s.path) != s.config->locations.end())
						s.rules = &(s.config->locations.at(s.path));
					}
				}
				else
					s.query = buffer.substr(i-len+1, len).cppstring();
				cerr << s.path << endl;
				if (s.rules == NULL)
					throw(statusCodeException(404, "Not Found"));
				s.path.erase(s.path.end()-1);
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
	return -1;
}
