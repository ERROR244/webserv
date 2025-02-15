#include "server.h"

httpSession::httpSession(int clientFd, configuration* config) : config(config), req(Request(*this)), res(Response(*this)), cgi(NULL), statusCode(200), codeMeaning("OK") {}

httpSession::httpSession() : config(NULL), req(Request(*this)), res(Response(*this)), cgi(NULL), statusCode(200), codeMeaning("OK") {}

void	httpSession::reSetPath(const string& newPath) {
	path = newPath;
}

void	sendError(const int clientFd, const int statusCode, const string codeMeaning) {
	string msg;
	msg += "HTTP/1.1 " + to_string(statusCode) + " " + codeMeaning + "\r\n"; 
	msg += "Content-type: text/html\r\n";
	msg += "Transfer-Encoding: chunked\r\n";
	msg += "Connection: keep-alive\r\n";
	msg += "Server: bngn/0.1\r\n";
	msg += "\r\n";
	write(clientFd, msg.c_str(), msg.size());
	string body = "<!DOCTYPE html><html><body><h1>" + codeMeaning + "</h1></body></html>";
	ostringstream chunkSize;
	chunkSize << hex << body.size() << "\r\n";
	write(clientFd, chunkSize.str().c_str(), chunkSize.str().size());
	write(clientFd, body.c_str(), body.size());
	write(clientFd, "\r\n", 2);
	write(clientFd, "0\r\n\r\n", 5);
}


void	resSessionStatus(const int& epollFd, const int& clientFd, map<int, httpSession*>& s, const t_state& status) {
	struct epoll_event	ev;

	if (status == DONE) {
		ev.events = EPOLLIN;
		ev.data.fd = clientFd;
		if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev) == -1) {
			perror("epoll_ctl failed: ");
			throw(statusCodeException(500, "Internal Server Error"));
		}
		delete s[clientFd];
		s.erase(s.find(clientFd));
	}
	else if (status == CCLOSEDCON) {
		if (epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, &ev) == -1) {
			perror("epoll_ctl failed: ");
			throw(statusCodeException(500, "Internal Server Error"));//this throw is not supposed to be here
		}
		close(clientFd);
		delete s[clientFd];
		s.erase(s.find(clientFd));
	}
}

void	reqSessionStatus(const int& epollFd, const int& clientFd, map<int, httpSession*>& s, const t_state& status) {
	struct epoll_event	ev;

	if (status == DONE) {
		ev.events = EPOLLOUT;
		ev.data.fd = clientFd;
		if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev) == -1) {
			perror("epoll_ctl failed: ");
			throw(statusCodeException(500, "Internal Server Error"));
		}
	}
	else if (status == CCLOSEDCON) {
		if (epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, &ev) == -1) {
			perror("epoll_ctl failed: ");
			throw(statusCodeException(500, "Internal Server Error"));//this throw is not supposed to be here
		}
		close(clientFd);
		delete s[clientFd];
		s.erase(s.find(clientFd));
	}
}

void	acceptNewClient(const int& epollFd, const int& serverFd, const t_sockaddr& addrsInfo) {
	struct epoll_event	ev;
	int					clientFd;
	socklen_t			addrsLen = sizeof(addrsInfo);

	if ((clientFd = accept(serverFd, (struct sockaddr*)&addrsInfo, &addrsLen)) < 0) {
		perror("accept faield: ");
        throw(statusCodeException(500, "Internal Server Error"));//i can't send the error page//no fd to send to
    }
	ev.events = EPOLLIN;
	ev.data.fd = clientFd;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &ev) == -1) {
		perror("epoll_ctl faield(setUpserver.cpp): ");
		throw(statusCodeException(500, "Internal Server Error"));
	}
	cerr << "-------new client added-------" << endl;
}

void	multiplexerSytm(map<int, t_sockaddr>& servrSocks, const int& epollFd, map<string, configuration>& config) {
	struct epoll_event		events[MAX_EVENTS];
	map<int, httpSession*>	sessions;//change httpSession to a pointer so i can be able to free it

	while (1) {
		int nfds;
		cerr << "waiting for requests..." << endl;
		if ((nfds = epoll_wait(epollFd, events, MAX_EVENTS, -1)) == -1) {
			//send the internal error page to all current clients
			//close all connections and start over
			perror("epoll_wait failed(setUpserver.cpp): ");
		}
		for (int i = 0; i < nfds; ++i) {
			const int fd = events[i].data.fd;
			try {
				if (servrSocks.find(fd) != servrSocks.end())
					acceptNewClient(epollFd, fd, servrSocks[fd]);
				else if (events[i].events & EPOLLIN) {
					sessions.try_emplace(fd, new httpSession(fd, &(map[])));
					sessions[fd]->req.parseMessage(fd);
					reqSessionStatus(epollFd, fd, sessions, sessions[fd]->req.status());
				}
				else if (events[i].events & EPOLLOUT) {
					sessions[fd]->res.sendResponse(fd);
					resSessionStatus(epollFd, fd, sessions, sessions[fd]->res.status());
				}
			}
			catch (const statusCodeException& exception) {
				struct epoll_event	ev;
				cerr << "code--> " << exception.code() << endl;
				cerr << "reason--> " << exception.meaning() << endl;
				if (config.errorPages.find(exception.code()) != config.errorPages.end()) {
					sessions[fd]->reSetPath(w_realpath(("." + config.errorPages.at(exception.code())).c_str()));
					ev.events = EPOLLOUT;
					ev.data.fd = fd;
					if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev) == -1) {
						perror("epoll_ctl faield(setUpserver.cpp): "); exit(-1);
					}
				} else {
					sendError(fd, exception.code(), exception.meaning());
					ev.events = EPOLLIN;
					ev.data.fd = fd;
					if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev) == -1) {
						perror("epoll_ctl faield(setUpserver.cpp): "); exit(-1);
					}
					sessions.erase(sessions.find(fd));
				}
			}
		}
	}
}