#include "httpSession.hpp"

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
		case e_sstat::fieldLine: {
			switch (ch)
			{
			case ':': { //test-> : field name ? valid?
				fieldline = buffer.substr(pos-len, len).cppstring();
				sstat = e_sstat::wssBeforeFieldName;
				for (int i = 0; i < fieldline.size(); ++i)
					fieldline[i] = tolower(fieldline[i]);
				break;
			}
			case '-':
			case '_':
				break;
			default: {
				if (!iswalnum(ch))
					throw(statusCodeException(400, "Bad Request"));
			}
			}
			++len;
			break;
		}
		case e_sstat::wssBeforeFieldName: {
			switch (ch)
			{
			case '\r': {
				headers[fieldline] = "";
				sstat = e_sstat::fieldNl;
				break;
			}
			case '\n': {
				headers[fieldline] = "";
				sstat = e_sstat::emptyline;
				len = 0;
				++pos;
				continue;
			}
			case ' ': case '\t':
			case '\f': case '\v':
				break;
			default: {
				sstat = e_sstat::filedName;
				len = 0;
				continue;
			}
			}
			break;
		}
		case e_sstat::filedName: {
			switch (ch)
			{
			case '\r': {
				headers[fieldline] = buffer.substr(pos-len, len).cppstring();
				sstat = e_sstat::fieldNl;
				break;
			}
			case '\n': {
				headers[fieldline] = buffer.substr(pos-len, len).cppstring();
				sstat = e_sstat::emptyline;
				len = 0;
				++pos;
				continue;
			}
			}
			++len;
			break ;
		}
		case e_sstat::fieldNl: {
			if (ch != '\n')
				throw(statusCodeException(400, "Bad Request"));
			sstat = e_sstat::emptyline;
			len = 0;
			break;
		}
		case e_sstat::emptyline: {
			switch (ch)
			{
			case '\r': {
				if (len != 0)
					throw(statusCodeException(400, "Bad Request"));
				break;
			}
			case '\n': {
				switch (method)
				{
					case POST: {
						sstat = e_sstat::body;
						break;
					}
					default:
						sstat = e_sstat::sHeader;
				}
				cerr << "----headers----" << endl;
				for (const auto& it : headers)
					cerr << it.first << ": " << it.second << endl;
				cerr << "----headers----" << endl;
				return pos+1;
			}
			default: {
				if (len != 0)
					throw(statusCodeException(400, "Bad Request"));
				sstat = e_sstat::fieldLine;
				len = 0;
				continue;
			}
			}
			++len;
			break;
		}
		}
		++pos;
		if (++headerFieldsLen > HEADER_FIELD_MAXSIZE)
			throw(statusCodeException(431, "Request Header Fields Too Large"));
	}
	throw(statusCodeException(400, "Bad Request"));
	return -1;
}