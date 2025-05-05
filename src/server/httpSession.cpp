#include "httpSession.hpp"

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

map<string, string>	httpSession::getHeaders() {
	return headers;
}

void	httpSession::resetForSendingErrorPage(const string& errorPagePath) {
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
}

bstring&	httpSession::getCgiBody() {
	return cgiBody;
}

Cgi* httpSession::getCgi() {
	return cgi;
}
