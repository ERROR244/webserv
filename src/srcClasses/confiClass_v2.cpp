#include "confiClass.hpp"

string  trim(const string& str) {
	size_t  start = str.find_first_not_of(" \t\n\r\f\v");
	if (start == string::npos)  return "";
	size_t  end = str.find_last_not_of(" \t\n\r\f\v");
	return str.substr(start, end - start + 1);
}

int checkKey(string key, const string& line) {
    size_t     i;

    for (i = 0; i < key.size(); ++i) {
        if (key[i] != line[i]) {
            throw "expected: `" + key + "` got `" + line + "`";
        }
    }
    return (i);
}

int ft_stoi(const std::string &__str) {
    try {
        int res = stoi(__str);
        return (res);
    }
    catch (exception& e) {
        throw "unvalid number: `" + __str + "`";
    }
}

bool isNumber(const std::string& str) {  
    for (char c : str) {
        int v = c;
        if (!(c >= 48 && c <= 57)) {
            return false;
        }
    }
    if (65536 < ft_stoi(str)) {
        throw "port bigger than 65536.";
    }
    return true;
}

void handlePort(string& line, int len, keyValue& kv, ifstream& sFile) {
    int i = checkKey("port:", line);

    line = trim(line.substr(i));
    if (isNumber(line) == false) {
        throw "invalid port.";
    }
    kv.port = line;
}

string getCurrentHost(keyValue& kv, string line) {
    if (line.empty())
        throw "host can't be empty";
    kv.addInfo = NULL;
    getaddrinfo(line.c_str(), kv.port.c_str(), NULL, &kv.addInfo);
    if (kv.addInfo == NULL)
        throw "kv.addInfo is NULL";
    return line;
}

void handlehost(string& line, int len, keyValue& kv, ifstream& sFile) {
    int i = checkKey("host:", line);
    string tmp;

    line = trim(line.substr(i));
    kv.host = getCurrentHost(kv, line);
    if (kv.host.empty())
        throw "host can't be empty";
}

void handleSerNames(string& line, int len, keyValue& kv, ifstream& sFile) {
    int i = checkKey("serN:", line);
    string tmp;

    line = trim(line.substr(i));
    while (true) {
        i = line.find_first_of(',');
        if (i == string::npos) {
            kv.serNames.push_back(trim(line));
            break;
        }
        tmp = line.substr(0, i);
        kv.serNames.push_back(trim(tmp));
        line = line.substr(i);
        if (line[0] == ',')
            line = line.substr(1);
    }
}

void handleBodyLimit(string& line, int len, keyValue& kv, ifstream& sFile) {
    int i = checkKey("body:", line);
    kv.bodySize = ft_stoi(trim(line.substr(i)));
}

void handleCgi(string& line, int len, keyValue& kv, ifstream& sFile) {
    pair<string, string>    holdValue;
    int                     index = 0;
    int                     i = 0;

    if (trim(line) != "[cgi]") {
        throw "expected: `[cgi]` got `" + line + "`";
    }

    while (getline(sFile, line)) {
        if (trim(line) == "[END]") { return ; }
        else if (i == 2) { break; }
        line = trim(line);
        if (line[0] == 'A') {
            index = checkKey("Alias-script:", trim(line));
            line = trim(trim(line).substr(index));
            if (line.empty())
                throw "alias-script can't be empty";
            holdValue.first = trim(line.substr(0, index));
            holdValue.second = "";
            kv.cgis["alias-script"].push_back(holdValue);
        }
        else {
            index = checkKey("add-handler:", line);
            line = trim(line.substr(index));
            index = line.find_first_of(' ');
            if (line.empty())
                throw "add-handler can't be empty";
            else if (index == string::npos)
                throw "add-handler need two values";
            holdValue.first = trim(line.substr(0, index));
            holdValue.second = trim(line.substr(index));
            kv.cgis["add-handler"].push_back(holdValue);
        }
    }
}

void handleError(string& line, int len, keyValue& kv, ifstream& sFile) {
    pair<int, string> holdValue;
    if (trim(line) != "[errors]") {
        throw "expected: `[errors]` got `" + line + "`";
    }
    while (getline(sFile, line)) {
        if (trim(line) == "[END]") { return ; }
        holdValue.first = ft_stoi(trim(line).substr(0, 3));
        holdValue.second = trim(line).substr(4);
        kv.errorPages.push_back(holdValue);
    }
}

/////////////////////// ROOTS

void handleUrl(string& line, root& kv, ifstream& sFile) {
    int i = checkKey("url:", line);
    kv.url = trim(line.substr(i));
    if (kv.url.empty())
        throw "root url can't be empty";
}

void handleAlias(string& line, root& kv, ifstream& sFile) {
    int i = checkKey("alias:", line);
    kv.alias = trim(line.substr(i));
    if (kv.alias.empty())
        throw "root alias can't be empty";
}

void handleMethods(string& line, root& kv, ifstream& sFile) {
    int i = checkKey("methods:", line);
    string tmp;

    line = trim(line.substr(i));
    while (true) {
        i = line.find_first_of(',');
        if (i == string::npos) {
            if (!trim(tmp).empty())
                kv.methods.push_back(trim(line));
            break;
        }
        tmp = line.substr(0, i);
        if (!trim(tmp).empty())
            kv.methods.push_back(trim(tmp));
        line = line.substr(i);
        if (line[0] == ',')
            line = line.substr(1);
    }
    if (kv.methods.size() > 3)
        throw "invalid numbers of methods: " + to_string(kv.methods.size());
    for (int i = 0; i < kv.methods.size(); ++i) {
        if (kv.methods[i] != "GET" && kv.methods[i] != "DELETE" && kv.methods[i] != "POST")
            throw "invalid method: `" + kv.methods[i] + "`";
    }
    if (kv.methods.empty())
        kv.methods = {"GET", "DELETE", "POST"};
}

void handleIndex(string& line, root& kv, ifstream& sFile) {
    int i = checkKey("Index:", line);
    kv.index = trim(line.substr(i));
    if (kv.index.empty())
        kv.index = "index.html";
}

void handleAutoIndex(string& line, root& kv, ifstream& sFile) {
    int i = checkKey("autoIndex:", line);
    line = trim(line.substr(i));

    if (line == "on")
        kv.autoIndex = true;
    else
        kv.autoIndex = false;
}

root handleRoot(ifstream& sFile) {
    void (*farr[])(string& line, root& kv, ifstream& sFile) = {handleUrl, handleAlias, handleMethods, handleIndex, handleAutoIndex};
    string line;
    root kv;
    int i = 0;

    while (getline(sFile, line)) {
        line = trim(line);
        if (line == "[END]") {
            return kv;
        }
        else if (line.empty())
            continue;
        if (i > 4)
            break;
        farr[i](line, kv, sFile);
        i++;
    }
    throw "[END] tag neede";
}

void handlelocs(string& line, int len, keyValue& kv, ifstream& sFile) {
    if (line != "[ROOTS]")
            throw "handlelocs::unknown keywords: `" + line + "`";
    while (getline(sFile, line)) {
        line = trim(line);
        if (line.empty())
            continue;
        if (line == "[root]") {
            kv.roots.push_back(handleRoot(sFile));
        }
        else if (line == "[END]") {
            break ;
        }
        else {
            throw "handlelocs::unknown keywords: `" + line + "`";
        }
    }
}
