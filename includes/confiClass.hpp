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

struct root {
    vector<string>  methods;
    string          url;
    string          alias;
    string          index;
    bool            autoIndex;
};

struct keyValue {
    int                                         bodySize;
    string                                      port;
    string                                      host;
    vector<string>                              serNames;
    vector<pair<int, string>>                   errorPages;
    map<string, vector<pair<string, string>>>   cgis;
    vector<root>                                roots;
    struct addrinfo*                            addInfo;
};


class confiClass {
    private:
        string              file;

    
    public:
        map<int, keyValue>  kValue;

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
