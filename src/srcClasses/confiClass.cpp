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

keyValue confiClass::handleServer(ifstream& sFile) {
    void (*farr[])(string& line, int len, keyValue& kv, ifstream& sFile) = {handlePort, handlehost, handleSerNames, handleBodyLimit, handleError, handleCgi, handlelocs};
    string line;
    keyValue kv;
    int i = 0;

    while (getline(sFile, line)) {
        line = trim(line);
        if (line == "[END]")
            return kv;
        else if (line.empty())
            continue;
        if (i > 6)
            break;
        farr[i](line, i, kv, sFile);
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

