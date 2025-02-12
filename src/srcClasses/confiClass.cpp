#include "confiClass.hpp"

confiClass::confiClass() { }

confiClass::confiClass(string _file) {
    file = _file;
}
confiClass::~confiClass() {
    map<string, keyValue>::iterator it;

    for (it = kValue.begin(); it != kValue.end(); ++it) {
        freeaddrinfo(it->second.addInfo);
    }
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
    else if (line[0] == '[' && line[1] == 'c')
        return CGI;
    else if (line[0] == '[' && line[1] == 'R')
        return LOCS;
    else
        // cout << line[0] << endl;
        throw "line can't be empty";
}

keyValue confiClass::handleServer(ifstream& sFile) {
    void (*farr[])(string& line, int len, keyValue& kv, ifstream& sFile) = {handlePort, handlehost, handleSerNames, handleBodyLimit, handleError, handleCgi, handlelocs};
    string line;
    keyValue kv;
    int index;
    int i = 0;

    while (getline(sFile, line)) {
        line = trim(line);
        if (line == "[END]")
            return kv;
        else if (line.empty())
            continue;
        if (i > 6)
            break;
        index = getSer1(line);
        farr[index](line, i, kv, sFile);
        i++;
    }
    throw "[END] tag neede";
}

void confiClass::parseFile() {
    ifstream    sFile(file);
    keyValue    kv;
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
            kv = handleServer(sFile);
        }
        else {
            throw "parseFile::unknown keywords: `" + line + "`";
        }
        key = kv.host + ":" + kv.port;
        if (kValue.find(key) == kValue.end())
            kValue[key] = kv;
    }
    sFile.close();
}

