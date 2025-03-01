#pragma once
#include <string>
#include <exception>

using namespace std;

class statusCodeException : public exception {
	const int 		_code;
	const string	_meaning;
	public:
		statusCodeException(const int code, const string meaning);
		const int		code() const;
		const string	meaning() const;
		// virtual const char* what() const throw();
		virtual ~statusCodeException() throw();

};