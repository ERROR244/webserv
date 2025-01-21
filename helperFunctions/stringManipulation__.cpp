#include <iostream>
#include <vector>

using namespace std;

string  trim(const string& str) {
	size_t  start = str.find_first_not_of(" \t\n\r\f\v");
	if (start == string::npos)  return "";

	size_t  end = str.find_last_not_of(" \t\n\r\f\v");

	return str.substr(start, end - start + 1);
}

vector<string>	split_ws(string& str) {
	const string	whiteSpace = " \t\n\r\f\v";
	string			remainingStr;
	int				pos = 0, i = 0;
	vector<string>	strings;

	while (str[i]) {
		if (whiteSpace.find(str[i]) != string::npos) {
			strings.push_back(str.substr(pos, i - pos));
			while (str[i] && whiteSpace.find(str[i]) != string::npos)	++i;
			pos = i;
		}
		else ++i;
	}
	while (str.begin()+pos != str.end()) {
		remainingStr += *(str.begin()+pos); //movin the string to point to what i ll add to the remaining buffer;
		++pos;
	}
	str = remainingStr;
	return strings;
}