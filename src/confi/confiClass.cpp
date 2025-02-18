#include "confiClass.hpp"

ConfigFileParser::ConfigFileParser() { }

ConfigFileParser::ConfigFileParser(string _file) {
    file = _file;
}
ConfigFileParser::~ConfigFileParser() {
    map<string, configuration>::iterator it;
    
    for (it = kValue.begin(); it != kValue.end(); ++it) {
        freeaddrinfo(it->second.addInfo);
        it->second.addInfo = NULL;
    }
    if (kv.addInfo != NULL)
        freeaddrinfo(kv.addInfo);
}

int getSer1(string line) {
    if (line[0] == 'p')
        return PORT;
    else if (line[0] == 'h')
        return HOST;
    else if (line[0] == 's')
        return SERNAMES;
    else if (line[0] == 'b')
        return BODYLIMIT;
    else if (checkRule(line, "errors"))
        return ERROR;
    else if (checkRule(line, "ROOTS"))
        return LOCS;
    else if (line.empty())
        throw "line can't be empty";
    throw "unexpected keyword: `" + line + "`";
}

void ConfigFileParser::handleServer(ifstream& sFile) {
    void (*farr[6])(string& line, configuration& kv, ifstream& sFile) = { handlePort,
                                                                         handlehost,
                                                                         handleSerNames,
                                                                         handleBodyLimit,
                                                                         handleError,
                                                                         handlelocs };
    int mp[6] = {0, 0, 0, 0, 0, 0};
    string line;
    int index;
    int i = 0;

    while (getline(sFile, line)) {
        line = trim(line);
        if (line == "}") {
            kv.addInfo = NULL;
            getaddrinfo(kv.host.c_str(), kv.port.c_str(), NULL, &kv.addInfo);
            if (kv.addInfo == NULL)
                throw "kv.addInfo is NULL";
            return ;
        }
        else if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;
        index = getSer1(line);
        if (mp[index] == -1) {
            throw "unexpected keyword: " + line;
        }
        mp[index] = -1;
        farr[index](line, kv, sFile);
        i++;
    }
    throw "`}` is expected at the end of each rule";
}

bool checkRule(string s1, string s2) {
    int i = 0;

    if (i < s1.size()) {
        while (i < s1.size() && i < s2.size()) {
            if (s1[i] != s2[i])
                return false;
            i++;
        }
        if (i != s2.size())
            return false;
        while (i < s1.size()) {
            if (s1[i] == '{')
                break;
            else if (s1[i] != 32 && s1[i] != 9) {
                return false;
            }
            i++;
        }
        if (i >= s1.size() || s1[i] != '{')
            return false;
    }
    return true;
}

map<string, configuration> ConfigFileParser::parseFile() {
    ifstream    sFile(file);
    string      line;
    string      key;

    if (!sFile) {
            throw "Unable to open file";
    }
    while (getline(sFile, line)) {
        line = trim(line);
        if (line.empty())
            continue;
        if (checkRule(line, "server")) {
            handleServer(sFile);
        }
        else {
            throw "parseFile::unknown keywords: `" + line + "`";
        }
        struct sockaddr_in *addr = (struct sockaddr_in *)kv.addInfo->ai_addr;
        char ipstr[INET_ADDRSTRLEN];

        inet_ntop(kv.addInfo->ai_family, &(addr->sin_addr), ipstr, sizeof(ipstr));
        key = string(ipstr) + ":" + kv.port;
        if (kValue.find(key) == kValue.end()) {
            kValue[key] = kv;
            kv.addInfo = NULL;
        }
    }
    sFile.close();
    return kValue;
}

