#pragma once
#include <string>
#include <exception>

using namespace std;

typedef enum e_state{
	PROCESSING,
	SHEADER,
	DONE,
	CCLOSEDCON,
}	t_state;

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