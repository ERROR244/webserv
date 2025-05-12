#include "confiClass.hpp"

size_t checkKey(const string& key, const string& line) {
	if (line.size() < key.size()) {
		throw runtime_error("checkKey::expected: `" + key + "` need to be smaller than `" + line + "`");
	}
	for (size_t i = 0; i < key.size(); ++i) {
		if (key[i] != line[i]) {
			throw runtime_error("checkKey::expected: `" + key + "` instead of `" + line + "`");
		}
	}
	return (key.size());
}

bool isNumber(const string& str) {  
	for (size_t i = 0; i < str.size(); ++i) {
		if (!(str[i] >= 48 && str[i] <= 57)) {
			return false;
		}
	}
	if (65536 < ft_stoi(str)) {
		throw runtime_error("port bigger than 65536.");
	}
	return true;
}

void handlePort(string& line, configuration& kv, ifstream& sFile) {
	(void)sFile;
	if (isNumber(line) == false) {
		throw runtime_error("invalid port.");
	}
	kv.port = line;
}

string getCurrentHost(configuration& kv, string line) {
	(void)kv;
	if (line.empty())
		throw runtime_error("host can't be empty");
	return line;
}

void handleHost(string& line, configuration& kv, ifstream& sFile) {
	(void)sFile;
	kv.host = getCurrentHost(kv, line);
	if (kv.host.empty())
		throw runtime_error("host can't be empty");
}

void handleListen(string& line, configuration& kv, ifstream& sFile) {
	string host;
	string port;
	int first_occ;
	int i;
	
	i = checkKey("listen:", line);
	line = trim(line.substr(i));
	first_occ = line.find_first_of(':');
	host = trim(line.substr(0, first_occ));
	port = trim(line.substr(first_occ + 1));
	if (!port.empty())
		handlePort(port, kv, sFile);
	if (!host.empty())
		handleHost(host, kv, sFile);
}

bool isValidDirectory(const char* path) {  
	struct stat s;

	if (stat(path, &s) == 0) {
		return S_ISDIR(s.st_mode);
	}
	return false;
}

void handleSerNames(string& line, configuration& kv, ifstream& sFile) {
	(void)sFile;
	string tmp;
	size_t i;
	
	i = checkKey("server_name:", line);
	line = trim(line.substr(i));
	while (true) {
		i = line.find_first_of(',');
		if (i == string::npos) {
			kv.serNames.push_back(trim(line));
			break;
		}
		tmp = trim(line.substr(0, i));
		kv.serNames.push_back(tmp);
		line = line.substr(i);
		if (line[0] == ',')
			line = line.substr(1);
	}
}

string getRPath(string path) {
	char    buf[PATH_MAX];
	char    *uploads;

	uploads = realpath(path.c_str(), buf);
	
	if (uploads == NULL)
		throw runtime_error("invalid path: `" + path + "`");
	return string(uploads);
}

off64_t ft_off64(const string& s) {
    const string max_str = "9223372036854775807";
    size_t len = s.length();

    if (len == 0)
        throw runtime_error("empty number");

    for (size_t i = 0; i < len; ++i) {
        if (s[i] < '0' || s[i] > '9')
            throw runtime_error("invalid character in number");
    }

    // Fast overflow check by comparing strings
    if (len > max_str.length() || (len == max_str.length() && s > max_str))
        throw runtime_error("value exceeds OFF64_MAX");

    off64_t result = 0;
    for (size_t i = 0; i < len; ++i) {
        int digit = s[i] - '0';
        result = result * 10 + digit;
    }

    return result;
}

void handleBodyLimit(string& line, configuration& kv, ifstream& sFile) {
    (void)sFile;
    int i = checkKey("limit_req:", line);
    line = trim(line.substr(i));

    try {
        kv.bodySize = ft_off64(line);
    } catch (const exception& e) {
        throw runtime_error("invalid limit_req: `" + line + "`: " + e.what());
    }
}

void handleError(string& line, configuration& kv, ifstream& sFile) {
	if (!checkRule(line, "errors")) {
		throw runtime_error("expected: `errors && {` got `" + line + "`");
	}
	while (getline(sFile, line)) {
		line = trim(line);
		if (line == "}") { return ; }
		else if (line.empty() || line[0] == '#' || line[0] == ';')
			continue;
		// kv.errorPages[ft_stoi(line.substr(0, 3))] = line.substr(4);
		kv.errorPages[ft_stoi(line.substr(0, 3))] = getRPath(trim(line.substr(4)));
	}
}


string  trim(const string& str) {
	size_t  start = str.find_first_not_of(" \t\n\r\f\v");
	if (start == string::npos)  return "";

	size_t  end = str.find_last_not_of(" \t\n\r\f\v");

	return str.substr(start, end - start + 1);
}

void handleAliasRed(string& line, location& kv, ifstream& sFile) {
	(void)sFile;
	int i;
	
	if (line[0] == 'a') {
		kv.redirection = false;
		i = checkKey("alias:", line);
	}
	else {
		kv.redirection = true;
		i = checkKey("return:", line);
	}
	kv.reconfigurer = trim(line.substr(i));
	if (kv.reconfigurer.empty())
		throw runtime_error("location alias can't be empty");
}

e_methods getMethods(const string& method) {
	if (method == "GET")
		return GET;
	else if (method == "DELETE")
		return DELETE;
	else if (method == "POST")
		return POST;
	return NONE;
}

string getMethods(e_methods method) {
	if (method == GET)
		return "GET";
	else if (method == DELETE)
		return "DELETE";
	else if (method == POST)
		return "POST";
	return "NONE";
}

void handleMethods(string& line, location& kv, ifstream& sFile) {
	(void)sFile;
	size_t i = checkKey("limit_except:", line);
	string tmp;

	line = trim(line.substr(i));
	if (line == "*" || line.empty()) {
		kv.methods.push_back(GET);
		kv.methods.push_back(POST);
		kv.methods.push_back(DELETE);
		return ;
	}
	while (true) {
		i = line.find_first_of(',');
		if (i == string::npos) {
			if (!trim(line).empty())
				kv.methods.push_back(getMethods(trim(line)));
			break;
		}
		tmp = line.substr(0, i);
		if (!trim(tmp).empty())
			kv.methods.push_back(getMethods(trim(tmp)));
		line = line.substr(i);
		if (line[0] == ',')
			line = line.substr(1);
	}
	if (kv.methods.size() > 3)
		throw runtime_error("invalid numbers of methods: " + toString(kv.methods.size()));
	for (size_t i = 0; i < kv.methods.size(); ++i) {
		if (kv.methods[i] != GET && kv.methods[i] != DELETE && kv.methods[i] != POST)
			throw runtime_error("invalid method at index: `" + toString(i) + "`");
	}
}

void handleIndex(string& line, location& kv, ifstream& sFile) {
	(void)sFile;
	int i = checkKey("index:", line);
	kv.index = trim(line.substr(i));
}

void handleUploads(string& line, location& kv, ifstream& sFile) {
	(void)sFile;
	int     i;
	
	i = checkKey("uploads:", line);
	kv.uploads = getRPath(trim(line.substr(i)));
}

void handleAutoIndex(string& line, location& kv, ifstream& sFile) {
	(void)sFile;
	int i = checkKey("autoindex:", line);
	line = trim(line.substr(i));

	kv.autoIndex = false;
	if (line == "on")
		kv.autoIndex = true;
}

bool isValidFile(const char* path) {
    struct stat s;
    if (stat(path, &s) == 0) {
        return S_ISREG(s.st_mode);
    }
    return false;
}

void handleCgi(string& line, location& kv, ifstream& sFile) {
	size_t  index = 0;
	string s1, s2;

	if (!checkRule(line, "cgi")) {
		throw runtime_error("expected: `cgi && {` instead of `" + line + "`");
	}
	while (getline(sFile, line)) {
		line = trim(line);
		if (line == "}") { return ; }
		else if (line.empty() || line[0] == '#' || line[0] == ';')
			continue;
		index = checkKey("add-handler:", line);
		line = trim(line.substr(index));
		index = line.find_first_of(' ');
		if (line.empty())
			throw runtime_error("add-handler can't be empty");
		else if (index == string::npos)
			throw runtime_error("add-handler need two values");
		s1 = trim(line.substr(0, index));
		s2 = trim(line.substr(index));
		kv.cgis[s1] = s2;
		if (!isValidFile(s2.c_str()))
			throw runtime_error(s2 + " doesn't exist");
	}
}

int getSer2(string line) {
	if (line.empty())
		throw runtime_error("line can't be empty");
	else if ((line[0] == 'a' && line[1] == 'l') || line[0] == 'r')
		return ALIASRRDI;
	else if (line[0] == 'l')
		return METHODS;
	else if (line[0] == 'i')
		return INDEX;
	else if (line[0] == 'a' && line[1] == 'u')
		return AUTOINDEX;
	else if (line[0] == 'u' && line[1] == 'p')
		return UPLOADS;
	else if (checkRule(line, "cgi"))
		return CGI;
	throw runtime_error("unexpected keyword: `" + line + "`");
}

string checkLocationRule(string s1, string s2) {
	size_t first_occ = s1.find_first_of(' ');
	size_t last_occ = s1.find_last_of(' ');


	if (first_occ == string::npos || last_occ == string::npos)
		throw runtime_error("checkLocationRule::unexpected keyword: ``" + s1 + "`" + "`");

	string key = trim(s1.substr(0, first_occ));
	string url = trim(s1.substr(first_occ, last_occ - first_occ));
	string closeBracket = trim(s1.substr(last_occ));

	if (key != s2)
		return "";
	else if (url.empty())
		return "";
	else if (closeBracket != "{")
		return "";
	return url;
}

void checkLocation(location& kv, int (&locationsFunc)[6]) {
	for (int i = 0; i < 6; ++i) {
		if (locationsFunc[i] == -1)
			continue;
		else if (i == 0) {
			kv.redirection = false;
			kv.reconfigurer = "/";
		}
		else if (i == 1){
			kv.methods.push_back(GET);
			kv.methods.push_back(POST);
			kv.methods.push_back(DELETE);
		}
		else if (i == 2)
			kv.index = "index.html";
		else if (i == 3)
			kv.autoIndex = false;
		else if (i == 5)
			kv.uploads = "";
	}
}

location handleLocation(ifstream& sFile, string line) {
	void (*farr[7])(string& line, location& kv, ifstream& sFile) = { handleAliasRed,
																	 handleMethods,
																	 handleIndex,
																	 handleAutoIndex,
																	 handleCgi,
																	 handleUploads };
	int locationsFunc[6] = {0};
	location    kv;
	int         index;

	kv.uri = line;
	while (getline(sFile, line)) {
		line = trim(line);
		if (line == "}") {
			checkLocation(kv, locationsFunc);
			return kv;
		}
		else if (line.empty() || line[0] == '#' || line[0] == ';')
			continue;
		index = getSer2(line);
		if (locationsFunc[index] == -1) {
			throw runtime_error("handleLocation::duplicate_is_error: `" + line + "`");
		}
		locationsFunc[index] = -1;
		farr[index](line, kv, sFile);
	}
	return location();
}

void handleLocs(string& line, configuration& kv, ifstream& sFile) {
	location rt;
	string url;

	if (!checkRule(line, "locations"))
			throw runtime_error("handlelocs::expected: `locations & {` instead of `" + line + "`");
	while (getline(sFile, line)) {
		line = trim(line);

		if (line.empty() || line[0] == '#' || line[0] == ';')
			continue;
		if (line == "}")
			return ;
	
		url = checkLocationRule(line, "location");
		if (url.empty()) {
			throw runtime_error("handlelocs::expected: `location & {` instead of `" + line + "`");
		}
		else {
			rt = handleLocation(sFile, url);
			kv.locations[rt.uri] = rt;
		}
	}
	throw runtime_error("`}` is expected at the end of each rule");
}
