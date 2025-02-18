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
    AUTOINDEX,  CGI,
    UPLOADS,    USRDIR
};

enum methods {
    GET,
    DELETE,
    POST,
    NONE
};


struct location {
    string                  url;
    string                  aliasRed;
    string                  index;
    string                  uploads;
    string                  usrDir;
    vector<methods>         methods;
    map<string, string>     cgis;
    bool                    isRed;
    bool                    autoIndex;
    location() :    index("index.html"),
                    uploads("uploads"),
                    usrDir("usrDir"),
                    autoIndex(false) {}
};

struct configuration {
    off_t                   bodySize;
    string                  port;
    string                  host;
    struct addrinfo*        addInfo;
    vector<string>          serNames;
    map<int, string>        errorPages;
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


void    handleSerNames(string& line, configuration& kv, ifstream& sFile);
void    handleBodyLimit(string& line, configuration& kv, ifstream& sFile);
void    handleError(string& line, configuration& kv, ifstream& sFile);
void    handlePort(string& line, configuration& kv, ifstream& sFile);
void    handlehost(string& line, configuration& kv, ifstream& sFile);
void    handlelocs(string& line, configuration& kv, ifstream& sFile);
void    handleUploads(string& line, location& kv, ifstream& sFile);
void    handleUsrDir(string& line, location& kv, ifstream& sFile);
void    printprint(map<string, configuration>);
bool    checkRule(string s1, string s2);
string  trim(const string& str);

#endif
