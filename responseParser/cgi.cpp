#include "requestParse.hpp"

bool	Request::isCGIScript() {
	string path(targetPath.begin()+1, targetPath.end());
	path += '/';
	stringstream ss(path);
	vector<string> strings;
	string line;

	while (getline(ss, line, '/'))
		strings.push_back(line);
	for (int i = 0; i < strings.size(); ++i) {
		//scripts dir
		if (strings[i] == configFile.getScriptFolder()) {
			//check next arg 
		}
		//scripts prefix
		for (const auto& it : configFile.getScriptPrefixes()) {
			if (strings[i].rfind("*it") != string::npos)
				
		}
	}
}