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
        throw runtime_error("line can't be empty");
    else if (line[0] == 'l' && line[1] == 'i' && line[2] == 's')
        return LISTEN;
    else if (line[0] == 's')
        return SERNAMES;
    else if (line[0] == 'l' && line[1] == 'i' && line[2] == 'm')
        return LIMIT_REQ;
    else if (checkRule(line, "errors"))
        return ERROR;
    else if (checkRule(line, "locations"))
        return LOCS;
    throw runtime_error("handleServer::getSer1::unexpected keyword: `" + line + "`");
}

void checkServer(configuration& kv, int (&serverFunc)[5]) {
    int locationsFunc[6] = {0};
    struct addrinfo hints;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    for (int i = 0; i < 5; ++i) {
        if (serverFunc[i] == -1)
            continue;
        else if (i == 0) {
            kv.host = "localhost";
            kv.port = "8080";
		}
        else if (i == 2) {
            kv.bodySize = 50;
		}
        else if (i == 4) {
			location defaultLoc;
			checkLocation(defaultLoc, locationsFunc);
			defaultLoc.uri = "/";
			kv.locations["/"] = defaultLoc;
		}
    }
	kv.addInfo = NULL;
    getaddrinfo(kv.host.c_str(), kv.port.c_str(), &hints, &kv.addInfo);
	if (kv.addInfo == NULL)
		throw runtime_error("handleServer::kv.addInfo is NULL");
}

void ConfigFileParser::handleServer(ifstream& sFile) {
    void (*funcArr[5])(string& line, configuration& kv, ifstream& sFile) = { handleListen,
                                                                          	 handleSerNames,
                                                                          	 handleBodyLimit,
                                                                          	 handleError,
                                                                          	 handleLocs };
    string  line;
    int     index = 0;
    int		serverFunc[5] = {0};

    while (getline(sFile, line)) {
        line = trim(line);
        if (line == "}") {
            checkServer(kv, serverFunc);
			return ;
        }
        else if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;
        index = getSer1(line);
        if (serverFunc[index] == -1) {
            throw runtime_error("handleServer::unexpected keyword: `" + line + "`");
        }
        serverFunc[index] = -1;
        funcArr[index](line, kv, sFile);
    }
    throw runtime_error("handleServer::`}` is expected at the end of each rule");
}

string getMapName(configuration &kv) {
	char		ipstr[INET_ADDRSTRLEN];
	struct		sockaddr_in *addr;
	string		key;

	addr = (struct sockaddr_in *)kv.addInfo->ai_addr;
	inet_ntop(kv.addInfo->ai_family, &(addr->sin_addr), ipstr, sizeof(ipstr));
	key = string(ipstr) + ":" + kv.port;
	return key;
}

map<string, configuration> ConfigFileParser::parseFile() {
    ifstream    sFile(file.c_str());
    string      line;
    string      key;
	int			serverFunc[5];

    if (!sFile) {
        throw runtime_error("Unable to open file");
    }
    while (getline(sFile, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;
        if (checkRule(line, "server")) {
            handleServer(sFile);
        }
        else {
            throw runtime_error("parseFile::expected: `server & {` need to be smaller than `" + line + "`");
        }
        key = getMapName(kv);
        if (kValue.find(key) == kValue.end()) {
            kValue[key] = kv;
            kv.addInfo = NULL;
        } 
        else {
            freeaddrinfo(kv.addInfo);
        }
        kv = configuration();
    }
	if (kValue.empty()) {
		checkServer(kv, serverFunc);
		kValue[getMapName(kv)] = kv;
		kv = configuration();
	}
    return kValue;
}


bool checkRule(string s1, string s2) {
    size_t first_occ = s1.find_first_of(' ');
    size_t last_occ = s1.find_last_of(' ');

    if (first_occ == string::npos || last_occ == string::npos)
        throw runtime_error("checkRule::unexpected keyword: `" + s1 + "`");

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

