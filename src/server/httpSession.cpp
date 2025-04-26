#include "httpSession.hpp"

httpSession::httpSession(int clientFd, configuration& config)
	: clientFd(clientFd), sstat(ss_method), config(config), rules(NULL)
	, cgi(NULL), showDirFiles(false), statusCode(200), codeMeaning("OK"), req(Request(*this)), res(Response(*this))
{
	// cerr << clientFd << " http session constructor called" << endl;
}

httpSession::httpSession()
	: clientFd(-1), sstat(ss_method), config(configuration()), rules(NULL)
	, cgi(NULL), showDirFiles(false), statusCode(200), codeMeaning("OK"), req(Request(*this)), res(Response(*this))
{
	// cerr << clientFd << " http session default constructor called" << endl;
}

httpSession::httpSession(const httpSession& other) : clientFd(other.clientFd), req(Request(*this)), res(Response(*this)) {
	// cerr << clientFd << " http session copy constructor called" << endl;
	sstat = other.sstat;
	method = other.method;
	path = other.path;
	query = other.query;
	headers = other.headers;
	config = other.config;
	rules = other.rules ? new location(*other.rules) : NULL;
	cgi = other.cgi ? new Cgi(*other.cgi) : NULL;
	cgiBody = other.cgiBody;
	returnedLocation = other.returnedLocation;
	showDirFiles = other.showDirFiles;
	statusCode = other.statusCode;
	codeMeaning = other.codeMeaning;
}

httpSession::~httpSession() {
	// cerr << clientFd << " http session destructor called" << endl;
	// delete[] rules;
	// delete[] cgi;
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

void	httpSession::resetForSendingErrorPage(const string& errorPagePath) {
	sstat = ss_sHeader;
	method = GET;
	path = errorPagePath;
	query = "";
	headers.clear();
	config = configuration();
	// if (rules)
	// 	delete rules;
	// if (cgi)
	// 	delete cgi;
	//why double free here HUHUHUHUHUHHUH
	rules = NULL;
	cgi = NULL;
	cgiBody = "";
	returnedLocation = "";
	showDirFiles = false;
}