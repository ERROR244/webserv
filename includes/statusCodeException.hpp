#pragma once
#include <string>
#include <exception>

using namespace std;

class statusCodeException : public exception {
	private:
		const int 		_code;
		const string	_meaning;
	public:
		statusCodeException(const int code, const string meaning);
		int		code() const;
		string	meaning() const;
		virtual ~statusCodeException() throw();

};