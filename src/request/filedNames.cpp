#include "httpSession.hpp"

inline std::string& trim(std::string& s) {
	s.erase(s.find_last_not_of(" \t\n\r\f\v") + 1);
	s.erase(0, s.find_first_not_of(" \t\n\r\f\v"));
    return s;
}

bool    httpSession::Request::validFieldName(string& str) const {
	for (auto& c: str) {
		if (!iswalnum(c) && c != '_' && c != '-')	return false;
		c = tolower(c);
	}
	return true;
}

bool	httpSession::Request::parseFileds(stringstream& stream) {
	string			line;

	while(getline(stream, line) && line != " ") {
		string	fieldName;
		string	filedValue;

		if (!s.headers.empty() && (line[0] == ' ' || line[0] == '\t')) {
			s.headers[prvsFieldName] += " " + trim(line);
			continue ;
		}

		size_t colonIndex = line.find(':');
		fieldName = line.substr(0, colonIndex);
		if (colonIndex != string::npos && colonIndex+1 < line.size()) {
			filedValue = line.substr(colonIndex+1);
			filedValue = trim(filedValue);
		}
		if (colonIndex == string::npos || !validFieldName(fieldName)) {
			throw(statusCodeException(400, "Bad Request"));
		}
		s.headers[fieldName] = filedValue;
		prvsFieldName = fieldName;
	}
	if (stream.eof()) {
		remainingBuffer = line;
		return false;
	}
	return true;
}