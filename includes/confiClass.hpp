# ifndef CONFICLASS_HPP
# define CONFICLASS_HPP

using namespace std;

# include <iostream>
# include <sstream>
# include <fstream>
# include <cstring>
# include <vector>
# include <map>
# include <algorithm>
# include <unistd.h>
# include <limits.h>
# include <cerrno>
# include <arpa/inet.h>
#include <netdb.h>

enum  Ser1 {
    PORT,       HOST,
    SERNAMES,   BODYLIMIT,
    ERROR,      LOCS
};

enum  Ser2 {
    URL,        ALIASRRDI,
    METHODS,    INDEX,
    AUTOINDEX,  CGI
};


string  trim(const string& str);


struct location {
    string                  url;
    string                  aliasRed;
    string                  index;
    vector<string>          methods;
    map<string, string>     cgis;
    bool                    isRed;
    bool                    autoIndex;
    location() : index("index.html") {}
};

struct configuration {
    off_t               bodySize;
    string              port;
    string              host;
    struct addrinfo*    addInfo;
    vector<string>      serNames;
    map<int, string>    errorPages;
    map<string, location>	locations;
};


class ConfigFileParser {
    private:
        string              file;

    
    public:
        map<string, configuration>   kValue;
        configuration                kv;

        ConfigFileParser();
        ConfigFileParser(string _file);
        ~ConfigFileParser();

        map<string, configuration>   parseFile();
        void                    handleServer(ifstream& sFile);
        void                    printprint();
};


void    handlePort(string& line, configuration& kv, ifstream& sFile);
void    handlehost(string& line, configuration& kv, ifstream& sFile);
void    handleSerNames(string& line, configuration& kv, ifstream& sFile);
void    handlelocs(string& line, configuration& kv, ifstream& sFile);
void    handleError(string& line, configuration& kv, ifstream& sFile);
void    handleBodyLimit(string& line, configuration& kv, ifstream& sFile);
string  trim(const string& str);
// void    handleCgi(string& line, configuration& kv, ifstream& sFile);

#endif
