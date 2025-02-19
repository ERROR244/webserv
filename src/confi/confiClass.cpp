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
    if (line.empty())
        throw std::runtime_error("line can't be empty");
    else if (line[0] == 'p')
        return PORT;
    else if (line[0] == 'h')
        return HOST;
    else if (line[0] == 'r')
        return ROOT;
    else if (line[0] == 's')
        return SERNAMES;
    else if (line[0] == 'l' && line[1] == 'i')
        return LIMIT_REQ;
    else if (checkRule(line, "errors"))
        return ERROR;
    else if (checkRule(line, "locations"))
        return LOCS;
    throw std::runtime_error("handleServer::getSer1::unexpected keyword: `" + line + "`");
}

void ConfigFileParser::handleServer(ifstream& sFile) {
    void (*farr[8])(string& line, configuration& kv, ifstream& sFile) = { handlePort,
                                                                          handlehost,
                                                                          handleRoot,
                                                                          handleSerNames,
                                                                          handleBodyLimit,
                                                                          handleError,
                                                                          handlelocs };
    int mp[8] = {0};
    string line;
    int index;
    int i = 0;

    while (getline(sFile, line)) {
        line = trim(line);
        if (line == "}") {
            kv.addInfo = NULL;
            getaddrinfo(kv.host.c_str(), kv.port.c_str(), NULL, &kv.addInfo);
            if (kv.addInfo == NULL)
                throw std::runtime_error("handleServer::kv.addInfo is NULL");
            return ;
        }
        else if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;
        index = getSer1(line);
        if (mp[index] == -1) {
            throw std::runtime_error("handleServer::unexpected keyword: `" + line + "`");
        }
        mp[index] = -1;
        farr[index](line, kv, sFile);
        i++;
    }
    throw std::runtime_error("handleServer::`}` is expected at the end of each rule");
}

bool checkRule(string s1, string s2) {
    int first_occ = s1.find_first_of(' ');
    int last_occ = s1.find_last_of(' ');

    if (first_occ == string::npos || last_occ == string::npos)
        throw std::runtime_error("checkRule::unexpected keyword: `" + s1 + "`");

    string key = trim(s1.substr(0, first_occ));
    string separator = trim(s1.substr(first_occ, last_occ - first_occ));
    string closeBracket = trim(s1.substr(last_occ));

    if (key != s2)
        return false;
    else if (!separator.empty())
        return false;
    else if (closeBracket != "{")
        return false;
    return true;
}

map<string, configuration> ConfigFileParser::parseFile() {
    ifstream    sFile(file);
    string      line;
    string      key;

    if (!sFile) {
            throw std::runtime_error("Unable to open file");
    }
    while (getline(sFile, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;
        if (checkRule(line, "server")) {
            handleServer(sFile);
        }
        else {
            throw std::runtime_error("parseFile::unknown keywords: `" + line + "`");
        }
        struct sockaddr_in *addr = (struct sockaddr_in *)kv.addInfo->ai_addr;
        char ipstr[INET_ADDRSTRLEN];

        inet_ntop(kv.addInfo->ai_family, &(addr->sin_addr), ipstr, sizeof(ipstr));
        key = string(ipstr) + ":" + kv.port;
        if (kValue.find(key) == kValue.end()) {
            kValue[key] = kv;
            kv.addInfo = NULL;
        }
        kv = configuration();
    }
    sFile.close();
    return kValue;
}

