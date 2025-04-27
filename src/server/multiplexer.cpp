#include "server.h"

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
			//why not close the client fd when epoll ctl fail
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
		cerr << "accept failed" << endl;
		return;
    }
	ev.events = EPOLLIN;
	ev.data.fd = clientFd;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &ev) == -1) {
		cerr << "epoll_ctl failed" << endl;
		close(clientFd);
		return;
	}
	cerr << "new client added, its fd is -> " << clientFd << endl;
}

void	multiplexerSytm(const vector<int>& servrSocks, const int& epollFd, map<string, configuration>& config) {
	struct epoll_event					events[MAX_EVENTS];
	map<int, httpSession>				sessions;
	int									nfds;

	while (1) {
		if ((nfds = epoll_wait(epollFd, events, MAX_EVENTS, 0)) == -1) {
			cerr << "epoll_wait failed" << endl;
        	if (errno == ENOMEM) {
				//ENOMEM is set when there'snt enough memory left in device
        	    for (map<int, httpSession>::iterator it = sessions.begin(); it != sessions.end(); ++it)
        	        close(it->first);
        	    close(epollFd);
        	    return;
    	    }
			continue;
		}
		for (int i = 0; i < nfds; ++i) {
			const int fd = events[i].data.fd;
			try {
				if (find(servrSocks.begin(), servrSocks.end(), fd) != servrSocks.end())
					acceptNewClient(epollFd, fd);
				else if (events[i].events & EPOLLIN) {
					if (sessions.find(fd) == sessions.end()) {
						pair<int, httpSession> newclient(fd, httpSession(fd, config[ft_getsockname(fd)]));
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
				errorResponse(epollFd, fd, sessions, exception);
			}
		}
	}
	
}