#include "server.h"

httpSession::httpSession(int clientFd, configuration& config)
	: clientFd(clientFd), sstat(ss_method), config(config), rules(NULL)
	, cgi(NULL), statusCode(200), codeMeaning("OK"), closeAutoIndex(false), req(Request(*this)), res(Response(*this)) {
	// cerr << clientFd << " http session constructor called" << endl;
}

httpSession::httpSession()
	: clientFd(-1), sstat(ss_method), config(configuration()), rules(NULL)
	, cgi(NULL), statusCode(200), codeMeaning("OK"), closeAutoIndex(false), req(Request(*this)), res(Response(*this)) {
	// cerr << clientFd << " http session default constructor called" << endl;
	}

httpSession::httpSession(const httpSession& other) : clientFd(other.clientFd),
	req(Request(*this)), res(Response(*this))
{
	// cerr << clientFd << " http session copy constructor called" << endl;
	closeAutoIndex = other.closeAutoIndex;
	sstat = other.sstat;
	method = other.method;
	path = other.path;
	query = other.query;
	headers = other.headers;
	config = other.config;
	rules = other.rules ? new location(*other.rules) : NULL;
	cgi = other.cgi ? new Cgi(*other.cgi) : NULL;
	cgiBody = other.cgiBody;
	statusCode = other.statusCode;
	codeMeaning = other.codeMeaning;
	returnedLocation = other.returnedLocation;
}

httpSession::~httpSession() {
	// cerr << clientFd << " http session destructor called" << endl;
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
}

void	resSessionStatus(const int& epollFd, const int& clientFd, map<int, httpSession>& s, const e_sstat& status) {
	struct epoll_event	ev;

	if (status == ss_done) {
		cerr << "done sending the response" << endl;
		ev.events = EPOLLIN;
		ev.data.fd = clientFd;
		if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev) == -1)
			perror("epoll_ctl failed");
		// delete s[clientFd];
		s.erase(s.find(clientFd));
	}
	else if (status == ss_cclosedcon) {
		cerr << "client closed the connection" << endl;
		if (epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, &ev) == -1)
			perror("epoll_ctl failed");
		close(clientFd);
		// delete s[clientFd];
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
		// delete s[clientFd];
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

bool checkTimeOutForEachUsr(std::map<int, time_t> &timeOut) {
	map<int, time_t>::iterator			it;

	for (it = timeOut.begin(); it != timeOut.end(); ++it) {
		if (checkTimeOut(timeOut, it->first, it->second) == true) {
			return true;
		}
	}
	return false;
}

class A {
	const int aa;
	const int bb;
	public:
		A(int aa, int bb) : aa(aa), bb(bb) {}
};

void	multiplexerSytm(const vector<int>& servrSocks, const int& epollFd, map<string, configuration>& config) {
	struct epoll_event					events[MAX_EVENTS];
	map<int, httpSession>				sessions;
	map<string, string>					sessionStorage;
	map<int, time_t>					timeOut;
	int									nfds;

	cerr << "Started the server..." << endl;
	while (1) {
		if (checkTimeOutForEachUsr(timeOut) == true) {
			continue ;
		}
		if ((nfds = epoll_wait(epollFd, events, MAX_EVENTS, 0)) == -1) {
			perror("epoll_wait failed");
			if (errno == EBADF || errno == ENOMEM) {
				//close all client's connections
				for (map<int, httpSession>::iterator it = sessions.begin(); it != sessions.end(); ++it) {
					close(it->first);
				}
				close(epollFd);
				return;
			}
			continue;
		}
		for (int i = 0; i < nfds; ++i) {
			const int fd = events[i].data.fd;
			try {
				if (find(servrSocks.begin(), servrSocks.end(), fd) != servrSocks.end()) {
					acceptNewClient(epollFd, fd);
				}
				else if (events[i].events & EPOLLIN) {
					if (sessions.find(fd) == sessions.end()) {
						sessions.insert({fd, httpSession(fd, config[getsockname(fd)])});
					}
					sessions[fd].req.readfromsock();
					reqSessionStatus(epollFd, fd, sessions, sessions[fd].status());
				}
				else if (events[i].events & EPOLLOUT) {
					timeOut[fd] = sessions[fd].res.handelClientRes(fd);
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