#include <iostream>

using namespace std;

string  trim(const string& str) {
	size_t  start = str.find_first_not_of(" \t\n\r\f\v");
	if (start == string::npos)  return "";

	size_t  end = str.find_last_not_of(" \t\n\r\f\v");

	return str.substr(start, end - start + 1);
}
