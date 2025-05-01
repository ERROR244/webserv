#include "server.h"

static void	resSessionStatus(const int& epollFd, const int& clientFd, map<int, httpSession>& s, map<int, epollPtr>& epollPtrHolder, const e_sstat& status) {
	struct epoll_event	ev;

	if (status == ss_done) {
		map<string, string> headers = s[clientFd].getHeaders();
		if (headers.find("connection") == headers.end() || headers["connection"] != "keep-alive") {
			cerr << "closing the connection of -> " << clientFd << endl;
			if (epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, &ev) == -1)
				cerr << "epoll_ctl failed" << endl;
			close(clientFd);
			epollPtrHolder.erase(epollPtrHolder.find(clientFd));
		} else {
			ev.events = EPOLLIN;
			epollPtrHolder[clientFd].fd = clientFd;
			ev.data.ptr = &epollPtrHolder[clientFd];
			if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev) == -1) {
				cerr << "epoll_ctl failed" << endl;
				close(clientFd);
				epollPtrHolder.erase(epollPtrHolder.find(clientFd));
			}
		}
		s.erase(s.find(clientFd));
	}
	else if (status == ss_cclosedcon) {
		cerr << "closing the connection of -> " << clientFd << endl;
		if (epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, &ev) == -1)
			perror("epoll_ctl failed");
		close(clientFd);
		s.erase(s.find(clientFd));
		epollPtrHolder.erase(epollPtrHolder.find(clientFd));
	}
}

static void	reqSessionStatus(const int& epollFd, const int& clientFd, map<int, httpSession>& s, map<int, epollPtr>& epollPtrHolder, const e_sstat& status) {
	struct epoll_event	ev;

	if (status == ss_sHeader) {
		ev.events = EPOLLOUT;
		epollPtrHolder[clientFd].fd = clientFd;
		ev.data.ptr = &epollPtrHolder[clientFd];
		if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev) == -1) {
			cerr << "epoll_ctl failed" << endl;
			close(clientFd);
			s.erase(s.find(clientFd));
			epollPtrHolder.erase(epollPtrHolder.find(clientFd));
		}
	}
	else if (status == ss_cclosedcon) {
		cerr << "closing the connection of -> " << clientFd << endl;
		if (epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, &ev) == -1)
			cerr <<"epoll_ctl failed" << endl;
		close(clientFd);
		s.erase(s.find(clientFd));
		epollPtrHolder.erase(epollPtrHolder.find(clientFd));
	}
}

static void	acceptNewClient(const int& epollFd, const int& serverFd, map<int, epollPtr>& epollPtrHolder) {
	struct epoll_event	ev;
	int					clientFd;

	if ((clientFd = accept(serverFd, NULL, NULL)) < 0) {
		cerr << "accept failed" << endl;
		return;
    }
	ev.events = EPOLLIN;
	epollPtrHolder[clientFd].fd = clientFd;
	ev.data.ptr = &epollPtrHolder[clientFd];
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &ev) == -1) {
		cerr << "epoll_ctl failed" << endl;
		close(clientFd);
		epollPtrHolder.erase(epollPtrHolder.find(clientFd));
		return;
	}
	cerr << "new client added, its fd is -> " << clientFd << endl;
}

void	multiplexerSytm(const vector<int>& servrSocks, const int& epollFd, map<string, configuration>& config) {
	struct epoll_event					events[MAX_EVENTS];
	map<int, httpSession>				sessions;
	map<int, epollPtr>					epollPtrHolder;
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
			epollPtr	*ptr = static_cast<epollPtr*>(events[i].data.ptr);
			const int 	fd = ptr->fd;
			try {
				if (find(servrSocks.begin(), servrSocks.end(), fd) != servrSocks.end())
					acceptNewClient(epollFd, fd, epollPtrHolder);
				else if (events[i].events & EPOLLIN) {
					if (ptr->ptr) {
						readCgiOutput(events[i]);
					} else {
						if (sessions.find(fd) == sessions.end()) {
							pair<int, httpSession> newclient(fd, httpSession(fd, config[ft_getsockname(fd)]));
							sessions.insert(newclient);
						}
						sessions[fd].req.readfromsock();
						reqSessionStatus(epollFd, fd, sessions, epollPtrHolder, sessions[fd].status());
					}
				}
				else if (events[i].events & EPOLLOUT) {
					if (ptr->ptr) {
						if (writeBodyToCgi(events[i])) {
							epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
							close(fd);
						}
					} else {
						sessions[fd].res.handelClientRes(epollFd);
						resSessionStatus(epollFd, fd, sessions, epollPtrHolder, sessions[fd].status());
					}
				}
			}
			catch (const statusCodeException& exception) {
				errorResponse(epollFd, fd, sessions, epollPtrHolder, exception);
			}
		}
	}
	
}