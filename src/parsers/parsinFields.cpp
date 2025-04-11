#include "httpSession.hpp"

inline string  trimTrailinWs(const string& str) {
	size_t  end = str.find_last_not_of(" \t\n\r\f\v");
	return str.substr(0, end + 1);
}

// bool	httpSession::Request::checkNecessaryHeaders() {
// 	if (s.headers.find("host"))
// }

int httpSession::parseFields(const bstring& buffer, size_t pos, map<string, string>& headers) {
	size_t	size = buffer.size();
	size_t	len = 0;
	size_t	headerFieldsLen = 0;
	string	fieldline;
	char	ch;
	while (pos < size) {
		ch = buffer[pos];
		switch (sstat)
		{
		case ss_fieldLine: {
			switch (ch)
			{
			case ':': { //test-> : field name ? valid?
				fieldline = buffer.substr(pos-len, len).cppstring();
				sstat = ss_wssBeforeFieldName;
				for (size_t i = 0; i < fieldline.size(); ++i)
					fieldline[i] = tolower(fieldline[i]);
				break;
			}
			case '-':
			case '_':
				break;
			default: {
				if (!iswalnum(ch))
					throw(statusCodeException(400, "Bad Request15"));
			}
			}
			++len;
			break;
		}
		case ss_wssBeforeFieldName: {
			switch (ch)
			{
			case '\r': {
				headers[fieldline] = "";
				sstat = ss_fieldNl;
				break;
			}
			case '\n': {
				headers[fieldline] = "";
				sstat = ss_emptyline;
				len = 0;
				++pos;
				continue;
			}
			case ' ': case '\t':
			case '\f': case '\v':
				break;
			default: {
				sstat = ss_filedName;
				len = 0;
				continue;
			}
			}
			break;
		}
		case ss_filedName: {
			switch (ch)
			{
			case '\r': {
				headers[fieldline] = trimTrailinWs(buffer.substr(pos-len, len).cppstring());
				sstat = ss_fieldNl;
				break;
			}
			case '\n': {
				headers[fieldline] = trimTrailinWs(buffer.substr(pos-len, len).cppstring());
				sstat = ss_emptyline;
				len = 0;
				++pos;
				continue;
			}
			}
			++len;
			break ;
		}
		case ss_fieldNl: {
			if (ch != '\n')
				throw(statusCodeException(400, "Bad Request16"));
			sstat = ss_emptyline;
			len = 0;
			break;
		}
		case ss_emptyline: {
			switch (ch)
			{
			case '\r': {
				if (len != 0)
					throw(statusCodeException(400, "Bad Request17"));
				break;
			}
			case '\n': {
				switch (method)
				{
					case POST: {
						sstat = ss_body;
						break;
					}
					default:
						sstat = ss_sHeader;
				}
				// cerr << "----headers----" << endl;
				// for (const auto& it : headers)
				// 	cerr << it.first << ": " << it.second << "|" <<  endl;
				// cerr << "--------------" << endl;
				return pos+1;
			}
			default: {
				if (len != 0)
					throw(statusCodeException(400, "Bad Request18"));
				sstat = ss_fieldLine;
				len = 0;
				continue;
			}
			}
			++len;
			break;
		}
		default:
			break;
		}
		++pos;
		if (++headerFieldsLen > HEADER_FIELD_MAXSIZE)
			throw(statusCodeException(431, "Request Header Fields Too Large"));
	}
	return -1;
}