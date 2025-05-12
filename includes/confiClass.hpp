# ifndef CONFICLASS_HPP
# define CONFICLASS_HPP

# include <sys/stat.h>  
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
#include "wrappers.h"

#define OFF64_MAX 9223372036854775807LL

enum  Ser1 {
	LISTEN,		SERNAMES,
	LIMIT_REQ,	ERROR,
	LOCS
};

enum  Ser2 {
	ALIASRRDI,  METHODS,
	INDEX,	  AUTOINDEX,
	CGI,		UPLOADS
};

enum e_methods {
	GET,
	DELETE,
	POST,
	NONE
};


struct location {
	string				uri;
	string				reconfigurer;
	string				index;
	string				uploads;
	vector<e_methods>	methods;
	map<string, string>	cgis;
	bool				redirection;
	bool				autoIndex;
};

struct configuration {
	off64_t					bodySize;
	string					port;
	string					host;
	struct addrinfo*		addInfo;
	vector<string>			serNames;
	map<int, string>		errorPages;
	map<string, location>	locations;

	configuration() : addInfo(NULL) {}
};


class ConfigFileParser {
	private:
		string			  file;

		
	public:
		map<string, configuration>	kValue;
		configuration				kv;

		ConfigFileParser();
		ConfigFileParser(string _file);
		~ConfigFileParser();

		map<string, configuration>   parseFile();
		void					handleServer(ifstream& sFile);
};


void		handleSerNames(string& line, configuration& kv, ifstream& sFile);
void		handleBodyLimit(string& line, configuration& kv, ifstream& sFile);
void		handleError(string& line, configuration& kv, ifstream& sFile);
void		handlePort(string& line, configuration& kv, ifstream& sFile);
void		handleListen(string& line, configuration& kv, ifstream& sFile);
void		handleHost(string& line, configuration& kv, ifstream& sFile);
void		handleLocs(string& line, configuration& kv, ifstream& sFile);
void		handleUploads(string& line, location& kv, ifstream& sFile);
void		checkLocation(location& kv, int (&locationsFunc)[6]);
e_methods	getMethods(const string& method);
bool		checkRule(string s1, string s2);
string		getMethods(e_methods method);
string  	trim(const string& str);

#endif
