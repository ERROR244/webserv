#include "binarystring.hpp"

bstring::bstringExceptions::bstringExceptions(std::string msg) : msg(msg) {}

bstring::bstringExceptions::~bstringExceptions() throw() {}

const char* bstring::bstringExceptions::what() const throw() {
	return msg.c_str();
}
