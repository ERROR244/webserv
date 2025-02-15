#include "confiClass.hpp"

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

void handlePort(string& line, configuration& kv, ifstream& sFile) {
    int i = checkKey("port:", line);

    line = trim(line.substr(i));
    if (isNumber(line) == false) {
        throw "invalid port.";
    }
    kv.port = line;
}

string getCurrentHost(configuration& kv, string line) {
    if (line.empty())
        throw "host can't be empty";
    kv.addInfo = NULL;
    getaddrinfo(line.c_str(), kv.port.c_str(), NULL, &kv.addInfo);
    if (kv.addInfo == NULL)
        throw "kv.addInfo is NULL";
    return line;
}

void handlehost(string& line, configuration& kv, ifstream& sFile) {
    int i = checkKey("host:", line);
    string tmp;

    line = trim(line.substr(i));
    kv.host = getCurrentHost(kv, line);
    if (kv.host.empty())
        throw "host can't be empty";
}

void handleSerNames(string& line, configuration& kv, ifstream& sFile) {
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

void handleBodyLimit(string& line, configuration& kv, ifstream& sFile) {
    int i = checkKey("body:", line);
    kv.bodySize = ft_stoi(trim(line.substr(i)));
}

void handleError(string& line, configuration& kv, ifstream& sFile) {
    if (trim(line) != "[errors]") {
        throw "expected: `[errors]` got `" + line + "`";
    }
    while (getline(sFile, line)) {
        if (trim(line) == "[END]") { return ; }
        kv.errorPages[ft_stoi(trim(line).substr(0, 3))] = trim(line).substr(4);
    }
}


string  trim(const string& str) {
	size_t  start = str.find_first_not_of(" \t\n\r\f\v");
	if (start == string::npos)  return "";

	size_t  end = str.find_last_not_of(" \t\n\r\f\v");

	return str.substr(start, end - start + 1);
}


/////////////////////// ROOTS

void handleUrl(string& line, location& kv, ifstream& sFile) {
    int i = checkKey("url:", line);
    kv.url = trim(line.substr(i));
    if (kv.url.empty())
        throw "root url can't be empty";
}

void handleAliasRed(string& line, location& kv, ifstream& sFile) {
    int i;
    
    if (line[0] == 'a') {
        kv.red = false;
        i = checkKey("alias:", line);
    }
    else {
        kv.red = true;
        i = checkKey("redirection:", line);
    }
    kv.aliasRed = trim(line.substr(i));
    if (kv.aliasRed.empty())
        throw "root alias can't be empty";
}

void handleMethods(string& line, location& kv, ifstream& sFile) {
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

void handleIndex(string& line, location& kv, ifstream& sFile) {
    int i = checkKey("Index:", line);
    kv.index = trim(line.substr(i));
    if (kv.index.empty())
        kv.index = "index.html";
}

void handleAutoIndex(string& line, location& kv, ifstream& sFile) {
    int i = checkKey("AutoIndex:", line);
    line = trim(line.substr(i));

    if (line == "on")
        kv.autoIndex = true;
    else
        kv.autoIndex = false;
}


void handleCgi(string& line, location& kv, ifstream& sFile) {
    int                     index = 0;
    int                     i = 0;

    if (trim(line) != "[cgi]") {
        throw "expected: `[cgi]` got `" + line + "`";
    }

    while (getline(sFile, line)) {
        if (trim(line) == "[END]") { return ; }
        else if (i == 2) { break; }
        line = trim(line);
        index = checkKey("add-handler:", line);
        line = trim(line.substr(index));
        index = line.find_first_of(' ');
        if (line.empty())
            throw "add-handler can't be empty";
        else if (index == string::npos)
            throw "add-handler need two values";
        kv.cgis[trim(line.substr(0, index))] = trim(line.substr(index));
    }
}

int getSer2(string line) {
    if (line[0] == 'u')
        return URL;
    else if (line[0] == 'a' || line[0] == 'r')
        return ALIASRRDI;
    else if (line[0] == 'm')
        return METHODS;
    else if (line[0] == 'I')
        return INDEX;
    else if (line[0] == 'A')
        return AUTOINDEX;
    else if (line[0] == '[' && line[1] == 'c')
        return CGI;
    else if (line.empty())
        throw "line can't be empty";
    throw "unexpected keyword: `" + line + "`";
}

location handleRoot(ifstream& sFile) {
    void (*farr[])(string& line, location& kv, ifstream& sFile) = {handleUrl, handleAliasRed, handleMethods, handleIndex, handleAutoIndex, handleCgi};
    int mp[6] = {0, 0, 0, 0, 0, 0};
    string line;
    location kv;
    int i = 0;
    int index;

    while (getline(sFile, line)) {
        line = trim(line);
        if (line == "[END]") {
            return kv;
        }
        else if (line.empty())
            continue;
        if (i > 5)
            break;
        index = getSer2(line);
        if (mp[index] == -1) {
            throw "unexpected keyword: " + line;
        }
        farr[index](line, kv, sFile);
        i++;
    }
    throw "[END] tag neede";
}

void handlelocs(string& line, configuration& kv, ifstream& sFile) {
    if (line != "[ROOTS]")
            throw "handlelocs::unknown keywords: `" + line + "`";
    while (getline(sFile, line)) {
        line = trim(line);
        if (line.empty())
            continue;
        if (line == "[root]") {
            location rt = handleRoot(sFile);
            kv.locations[rt.url] = rt;
        }
        else if (line == "[END]") {
            break ;
        }
        else {
            throw "handlelocs::unknown keywords: `" + line + "`";
        }
    }
}



void ConfigFileParser::printprint() {
    map<string, configuration>::iterator it;
    map<int, string>::iterator it_errorPages;
    int i = 0;

    for (it = kValue.begin(); it != kValue.end(); ++it) {
        if (it != kValue.begin())
            cout << "\n\n                              ------------------------------------\n\n" << endl;
        cout << "------------------SERVER-" << i << "------------------" << endl;
        cout << "---------> Ports: " << it->second.port << endl;
        cout << "---------> hosts: " << it->second.host << endl;
        cout << "---------> Server Names:";
        for (size_t j = 0; j < it->second.serNames.size(); ++j) {
            cout << " " << it->second.serNames[j];
            if (j + 1 < it->second.serNames.size())
                cout << ",";
        }
        cout << endl << "---------> Body Size: " << it->second.bodySize << "M";
        cout << endl << "---------> Error Pages:" << endl;
        for (it_errorPages = it->second.errorPages.begin(); it_errorPages != it->second.errorPages.end(); ++it_errorPages) {
            cout << "------------------> " << it_errorPages->first << " | " << it_errorPages->second << endl;
        }
        cout << endl << "---------> ADDINFO:" << endl;
        cout << "---------------------------> ai_addr:       " << it->second.addInfo->ai_addr << endl;
        cout << "---------------------------> ai_protocol:   " << it->second.addInfo->ai_protocol << endl;
        cout << "---------------------------> ai_flags:      " << it->second.addInfo->ai_flags << endl;
        cout << endl << "---------> ROOTS:" << endl;
        map<string, location>::iterator rootIt;
        for (rootIt = it->second.locations.begin(); rootIt != it->second.locations.end(); ++rootIt) {
            cout << "------------------> ROOT:" << endl;
            cout << "---------------------------> url:       " << rootIt->second.url << endl;
            if (rootIt->second.red == false)
                cout << "---------------------------> alias:     " << rootIt->second.aliasRed << endl;
            else
                cout << "---------------------------> redir:     " << rootIt->second.aliasRed << endl;
            cout << "---------------------------> Methods:   ";
            for (size_t k = 0; k < rootIt->second.methods.size(); ++k) {
                cout << rootIt->second.methods[k];
                if (k + 1 < rootIt->second.methods.size())
                    cout << ", ";
            }
            cout << endl;
            cout << "---------------------------> index:     " << rootIt->second.index << endl;
            cout << "---------------------------> autoIndex: " << (rootIt->second.autoIndex ? "True" : "False") << endl;
            if (!rootIt->second.cgis.empty())
                cout << "---------> Cgi Scripts:" << endl;
            map<string, string>::iterator cgiIt;
            for (cgiIt = rootIt->second.cgis.begin(); cgiIt != rootIt->second.cgis.end(); ++cgiIt) {
                cout << "------------------> add-handler: " << cgiIt->first << " | " << cgiIt->second << endl;
            }
            cout << endl;
        }
        i++;
    }
}

