#include "wrappers.h"

string	w_realpath(const char * file_name) {
	char absolutePath[1024];

	if (realpath(file_name, absolutePath) == nullptr)
		throw(statusCodeException(404, "Not Found"));
	return string(absolutePath);
}