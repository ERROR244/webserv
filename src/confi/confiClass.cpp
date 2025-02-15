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
    else if (line[0] == '[' && line[1] == 'e')
        return ERROR;
    else if (line[0] == '[' && line[1] == 'R')
        return LOCS;
    else if (line.empty())
        throw "line can't be empty";
    throw "unexpected keyword: `" + line + "`";
}

void ConfigFileParser::handleServer(ifstream& sFile) {
    void (*farr[])(string& line, configuration& kv, ifstream& sFile) = {handlePort, handlehost, handleSerNames, handleBodyLimit, handleError, handlelocs};
    int mp[6] = {0, 0, 0, 0, 0, 0};
    string line;
    int index;
    int i = 0;

    while (getline(sFile, line)) {
        line = trim(line);
        if (line == "[END]")
            return ;
        else if (line.empty())
            continue;
        if (i > 5)
            break;
        index = getSer1(line);
        if (mp[index] == -1) {
            throw "unexpected keyword: " + line;
        }
        mp[index] = -1;
        farr[index](line, kv, sFile);
        i++;
    }
    throw "[END] tag neede";
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
        if (trim(line) == "[server]") {
            handleServer(sFile);
        }
        else {
            throw "parseFile::unknown keywords: `" + line + "`";
        }
        key = kv.host + ":" + kv.port;
        if (kValue.find(key) == kValue.end()) {
            kValue[key] = kv;
            kv.addInfo = NULL;
        }
    }
    sFile.close();
    return kValue;
}

