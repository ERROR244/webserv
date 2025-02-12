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
    PORT,     HOST,
    SERNAMES, BODYLIMIT,
    ERROR,    CGI,
    LOCS
};

enum  Ser2 {
    URL,      ALIASRRDI,
    METHODS,  INDEX,
    AUTOINDEX
};


struct root {
    vector<string>  methods;
    string          url;
    string          alias;
    string          index;
    bool            autoIndex;
};

struct keyValue {
    off_t                                       bodySize;
    string                                      port;
    string                                      host;
    struct addrinfo*                            addInfo;
    vector<string>                              serNames;
    map<int, string>                            errorPages;
    map<string, vector<pair<string, string>>>   cgis;
    map<string, root>                           roots;
};


class confiClass {
    private:
        string              file;

    
    public:
        map<string, keyValue>  kValue;

        confiClass();
        confiClass(string _file);
        ~confiClass();

        void parseFile();
        keyValue handleServer(ifstream& sFile);
        void printKeyValue();
};


void    handlePort(string& line, int len, keyValue& kv, ifstream& sFile);
void    handlehost(string& line, int len, keyValue& kv, ifstream& sFile);
void    handleSerNames(string& line, int len, keyValue& kv, ifstream& sFile);
void    handlelocs(string& line, int len, keyValue& kv, ifstream& sFile);
void    handleError(string& line, int len, keyValue& kv, ifstream& sFile);
void    handleBodyLimit(string& line, int len, keyValue& kv, ifstream& sFile);
void    handleCgi(string& line, int len, keyValue& kv, ifstream& sFile);
string  trim(const string& str);

#endif
