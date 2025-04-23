#include "server.h"

httpSession::httpSession(int clientFd, configuration& config)
	: clientFd(clientFd), sstat(ss_method), config(config), rules(NULL)
	, cgi(NULL), showDirFiles(false), statusCode(200), codeMeaning("OK"), req(Request(*this)), res(Response(*this)) {
	// cerr << clientFd << " http session constructor called" << endl;
}

httpSession::httpSession()
	: clientFd(-1), sstat(ss_method), config(configuration()), rules(NULL)
	, cgi(NULL), showDirFiles(false), statusCode(200), codeMeaning("OK"), req(Request(*this)), res(Response(*this)) {
	// cerr << clientFd << " http session default constructor called" << endl;
	}

httpSession::httpSession(const httpSession& other) : clientFd(other.clientFd),
	req(Request(*this)), res(Response(*this))
{
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

void	httpSession::reSetPath(const string& newPath) {
	path = newPath;
	method = GET;
	headers["connection"] = "close";
}

void	resSessionStatus(const int& epollFd, const int& clientFd, map<int, httpSession>& s, const e_sstat& status) {
	struct epoll_event	ev;

	if (status == ss_done) {
		cerr << "done sending the response" << endl;
		map<string, string> headers = s[clientFd].getHeaders();
		if (headers["connection"] != "keep-alive") {
			cerr << "client want to close connection after this operation" << endl;
			if (epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, &ev) == -1)
				perror("epoll_ctl failed");
			close(clientFd);
		} else {
			ev.events = EPOLLIN;
			ev.data.fd = clientFd;
			if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev) == -1)
				perror("epoll_ctl failed");
		}
		s.erase(s.find(clientFd));
	}
	else if (status == ss_cclosedcon) {
		cerr << "client closed the connection" << endl;
		if (epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, &ev) == -1)
			perror("epoll_ctl failed");
		close(clientFd);
		s.erase(s.find(clientFd));
	}
}

void	reqSessionStatus(const int& epollFd, const int& clientFd, map<int, httpSession>& s, const e_sstat& status) {
	struct epoll_event	ev;

	if (status == ss_sHeader) {
		ev.events = EPOLLOUT;
		ev.data.fd = clientFd;
		if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev) == -1) {
			perror("epoll_ctl failed");
			throw(statusCodeException(500, "Internal Server Error"));
		}
	}
	else if (status == ss_cclosedcon) {
		cerr << "client closed the connection" << endl;
		if (epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, &ev) == -1)
			perror("epoll_ctl failed");
		close(clientFd);
		s.erase(s.find(clientFd));
	}
}

void	acceptNewClient(const int& epollFd, const int& serverFd) {
	struct epoll_event	ev;
	int					clientFd;

	if ((clientFd = accept(serverFd, NULL, NULL)) < 0) {
		perror("accept faield");
        return;
    }
	ev.events = EPOLLIN;
	ev.data.fd = clientFd;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &ev) == -1) {
		perror("epoll_ctl faield");
		close(clientFd);
		return;
	}
	cerr << "--------------new client added--------------" << endl;
}

void	multiplexerSytm(const vector<int>& servrSocks, const int& epollFd, map<string, configuration>& config) {
	struct epoll_event					events[MAX_EVENTS];
	map<int, httpSession>				sessions;
	int									nfds;

	cerr << "Started the server..." << endl;
	while (1) {
		if ((nfds = ft_epoll_wait(epollFd, events, MAX_EVENTS, 0, sessions, epollFd)) == -1)
			return;
		for (int i = 0; i < nfds; ++i) {
			const int fd = events[i].data.fd;
			try {
				if (find(servrSocks.begin(), servrSocks.end(), fd) != servrSocks.end())
					acceptNewClient(epollFd, fd);
				else if (events[i].events & EPOLLIN) {
					if (sessions.find(fd) == sessions.end()) {
						pair<int, httpSession> newclient(fd, httpSession(fd, config[getsockname(fd)]));
						sessions.insert(newclient);
					}
					sessions[fd].req.readfromsock();
					reqSessionStatus(epollFd, fd, sessions, sessions[fd].status());
				}
				else if (events[i].events & EPOLLOUT) {
					sessions[fd].res.handelClientRes(fd);
					resSessionStatus(epollFd, fd, sessions, sessions[fd].status());
				}
			}
			catch (const statusCodeException& exception) {
				if (errorResponse(epollFd, exception, sessions[fd]) < 0)
					sessions.erase(sessions.find(fd));
			}
		}
	}
	
}