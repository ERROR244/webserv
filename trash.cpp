// #include "requestParse.hpp"
#include <iostream>
#include <string.h>
#include <unordered_map>
#include <vector>
#include <stack>
#include <sstream>
#include <algorithm>
#include <unistd.h>
#include <arpa/inet.h>//for frecv

using namespace std;

#define ALIAS_SCRIPT "/bin/cgi"
#define CGI_HANDELER ".py"

void	isCGIScript(const string& str) {
	string path(str.begin()+1, str.end());//just to skip the first '/'
	stringstream ss(path);
	vector<string> strings;
	string line;
	while (getline(ss, line, '/')) {
		strings.push_back(line);
	}
	for (const auto& it : strings)
		cerr << it << endl;
	//scripts dir
	if (strncmp(str.c_str(), ALIAS_SCRIPT, strlen(ALIAS_SCRIPT)) == 0) {
		//check next arg
		cerr << "--CGI script--" << endl;
		return ;
	}
	for (int i = 0; i < strings.size(); ++i) {
		//scripts prefix
		// for (const auto& it : CGI_HANDELER) {
			if (strings[i].rfind(CGI_HANDELER) != string::npos) {
				cerr << "--CGI script--" << endl;
				return ;
			}
		// }
	}
	cerr << "-- not CGI script" << endl;
}

int main(int ac , char**av) {
	isCGIScript(string(av[1]));
}