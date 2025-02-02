#include "confiClass.hpp"

confiClass::confiClass() { }

confiClass::confiClass(string _file) {
    file = _file;
}
confiClass::~confiClass() {
    for (int i = 0; i < kValue.size(); ++i) {
        freeaddrinfo(kValue[i].addInfo);
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
    ifstream sFile(file);
    string line;
    int i = 0;
    keyValue kv;

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
        kValue[i++] = kv;
    }
    sFile.close();
}

