#pragma once

#include <unistd.h>
#include <string>
#include <string.h>
#include "statusCodeException.hpp"
#include <vector>
#include <iostream>
#include <exception>

class bstring {
protected:
	char*	__string;
	size_t	stringsize;
public:
	class bstringExceptions : std::exception {
	private:
		const std::string msg;
	public:
		virtual const char* what() const throw();
		virtual ~bstringExceptions() throw();
		bstringExceptions(std::string);
	};

	bstring();
	bstring(const char* str ,const size_t size);
	bstring(const bstring&);
	~bstring();
	const size_t&	size() const;
	const string	cppstring() const;
	const char*		c_str() const;
	bstring			substr(size_t start, size_t len=std::string::npos) const;
	vector<bstring>	split(const char* seperator = " ") const;
	bool			getheaderline(bstring&);
	bool			getline(bstring&);
	void			erase(const size_t start, size_t n);
	size_t			find(const char*, const size_t startpos = 0) const;
	size_t			find(const char, const size_t startpos = 0) const;
	size_t			rfind(const char, size_t startpos = std::string::npos) const;
	bstring			trimend(const char* = " \t\n\r\f\v") const;
	bool			cmp(const char* str) const;
	bool			ncmp(const char* str1, const size_t n, const size_t startpos = 0) const;
	bool			empty() const;
	const char* 	operator=(const char*);
	const bstring& 	operator=(const bstring&);
	const char& 	operator[](const size_t) const;
	const bstring&	operator+=(const bstring&);
	const char*		operator+=(const char*);
	const bstring	operator+(const bstring&);
};

std::ostream& operator<<(std::ostream &out, const bstring &fixed);