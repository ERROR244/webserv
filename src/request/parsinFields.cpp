#include "httpSession.hpp"

int httpSession::Request::parseFields(const bstring& buffer, size_t pos, map<string, string>& headers) {
	size_t	size = buffer.size();
	size_t	len = 0;
	size_t	headerFieldsLen = 0;
	string	fieldline;
	char	ch;

	while (pos < size) {
		ch = buffer[pos];
		switch (s.sstat)
		{
		case e_sstat::fieldLine: {
			switch (ch)
			{
			case ':': { //test-> : field name ? valid?
				fieldline = buffer.substr(pos-len, len).cppstring();
				s.sstat = e_sstat::wssBeforeFieldName;
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
				s.sstat = e_sstat::fieldNl;
				break;
			}
			case '\n': {
				headers[fieldline] = "";
				s.sstat = e_sstat::emptyline;
				len = 0;
				++pos;
				continue;
			}
			case ' ':
			case '\t':
			case '\f':
			case '\v':
				break;
			default: {
				s.sstat = e_sstat::filedName;
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
				s.sstat = e_sstat::fieldNl;
				break;
			}
			case '\n': {
				headers[fieldline] = buffer.substr(pos-len, len).cppstring();
				s.sstat = e_sstat::emptyline;
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
			s.sstat = e_sstat::emptyline;
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
				switch (s.method)
				{
					case POST: {
						s.sstat = e_sstat::bodyFormat;
						break;
					}
					default:
						s.sstat = e_sstat::sHeader;
				}
				return pos+1;
			}
			default: {
				if (len != 0)
					throw(statusCodeException(400, "Bad Request"));
				s.sstat = e_sstat::fieldLine;
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
	return -1;
}