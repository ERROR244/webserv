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


struct root {
    bool                    autoIndex;
    bool                    red;
    string                  url;
    string                  aliasRed;
    string                  index;
    vector<string>          methods;
    map<string, string>     cgis;
};

struct keyValue {
    off_t               bodySize;
    string              port;
    string              host;
    struct addrinfo*    addInfo;
    vector<string>      serNames;
    map<int, string>    errorPages;
    map<string, root>   roots;
};


class confiClass {
    private:
        string              file;

    
    public:
        map<string, keyValue>   kValue;
        keyValue                kv;

        confiClass();
        confiClass(string _file);
        ~confiClass();

        void        parseFile();
        void        handleServer(ifstream& sFile);
        void        printKeyValue();
};


void    handlePort(string& line, keyValue& kv, ifstream& sFile);
void    handlehost(string& line, keyValue& kv, ifstream& sFile);
void    handleSerNames(string& line, keyValue& kv, ifstream& sFile);
void    handlelocs(string& line, keyValue& kv, ifstream& sFile);
void    handleError(string& line, keyValue& kv, ifstream& sFile);
void    handleBodyLimit(string& line, keyValue& kv, ifstream& sFile);
string  trim(const string& str);
// void    handleCgi(string& line, keyValue& kv, ifstream& sFile);

#endif
