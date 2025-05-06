#include "httpSession.hpp"
#include "server.h"

httpSession::httpSession(int clientFd, configuration& config)
	: clientFd(clientFd), sstat(ss_method), config(config), rules(NULL)
	, cgi(NULL), showDirFiles(false), statusCode(200), codeMeaning("OK"), req(Request(*this)), res(Response(*this)) {}

httpSession::httpSession()
	: clientFd(-1), sstat(ss_method), config(configuration()), rules(NULL)
	, cgi(NULL), showDirFiles(false), statusCode(200), codeMeaning("OK"), req(Request(*this)), res(Response(*this)) {}

httpSession::httpSession(const httpSession& other) : clientFd(other.clientFd), req(Request(*this)), res(Response(*this)) {
	sstat = other.sstat;
	method = other.method;
	path = other.path;
	query = other.query;
	headers = other.headers;
	config = other.config;
	cgi = other.cgi ? new Cgi(*other.cgi) : NULL;
	rules = other.rules ? new location(*other.rules) : NULL;
	cgiBody = other.cgiBody;
	returnedLocation = other.returnedLocation;
	showDirFiles = other.showDirFiles;
	statusCode = other.statusCode;
	codeMeaning = other.codeMeaning;
}

httpSession::~httpSession() {
	delete cgi;
}

configuration	httpSession::clientConfiguration() const {
	return config;
}

int	httpSession::fd() const {
	return clientFd;
}

const e_sstat& httpSession::status() const {
	return sstat;
}

void	httpSession::closeCon() {
	sstat = ss_cclosedcon;
}

map<string, vector<string> >	httpSession::getHeaders() {
	return headers;
}

void	httpSession::resetForSendingErrorPage(const string& errorPagePath, int statusCode, string meaning) {
	map<int, epollPtr>&	monitor = getEpollMonitor();
	monitor[clientFd].timer = 0;
	sstat = ss_sHeader;
	method = GET;
	path = errorPagePath;
	query = "";
	headers.clear();
	config = configuration();
	if (cgi)
		delete cgi;
	cgi = NULL;
	cgiBody = "";
	returnedLocation = "";
	showDirFiles = false;
	this->statusCode = statusCode;
	this->codeMeaning = meaning;

}

bstring&	httpSession::getCgiBody() {
	return cgiBody;
}

Cgi* httpSession::getCgi() {
	return cgi;
}

string	getHeaderValue(map<string, vector<string> >& mp, const string& key) {
	if (mp.find(key) != mp.end()) {
		vector<string> value = mp.at(key);
		if (value.empty() == false)
			return value[0];
	}
	return "";
}