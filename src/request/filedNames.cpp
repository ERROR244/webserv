#include "httpSession.hpp"

bool    httpSession::Request::validFieldName(string& str) const {
	for (auto& c: str) {
		if (!iswalnum(c) && c != '_' && c != '-')	return false;
		c = tolower(c);
	}
	return true;
}

bool	httpSession::Request::parseFileds(bstring& buffer) {
	bstring	line;
	bool	eof;
	while((eof = buffer.getheaderline(line)) && !line.empty()) {
		string	fieldName;
		string	filedValue;

		if (!s.headers.empty() && (line[0] == ' ' || line[0] == '\t')) {
			s.headers[prvsFieldName] += " " + trim(line.cppstring());
			continue ;
		}
		size_t colonIndex = line.find(":");
		fieldName = line.substr(0, colonIndex).cppstring();
		if (colonIndex != string::npos && colonIndex+1 < line.size()) {
			filedValue = line.substr(colonIndex+1).cppstring();
			filedValue = trim(filedValue);
		}
		if (colonIndex == string::npos || !validFieldName(fieldName)) {
			throw(statusCodeException(400, "Bad Request"));
		}
		s.headers[fieldName] = filedValue;
		prvsFieldName = fieldName;
	}
	if (!eof) {
		remainingBuffer = line;
		return false;
	}
	return true;
}