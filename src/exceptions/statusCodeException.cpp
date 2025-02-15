#include "statusCodeException.hpp"

statusCodeException::statusCodeException(const int code, const string meaning) : _code(code), _meaning(meaning) {}

const int	statusCodeException::code() const {
	return _code;
}

const string	statusCodeException::meaning() const {
	return _meaning;
} 

statusCodeException::~statusCodeException() {}