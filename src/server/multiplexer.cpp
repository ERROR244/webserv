#include "server.h"

static void	resSessionStatus(const int& epollFd, const int& clientFd, map<int, httpSession>& s, const e_sstat& status) {
	struct epoll_event				ev;
	map<int, epollPtr>&				monitor = getEpollMonitor();
	map<int, epollPtr>::iterator	position = monitor.find(clientFd);

	if (status == ss_done) {
		map<string, vector<string> > headers = s[clientFd].getHeaders();
		if (headers.find("connection") == headers.end() || getHeaderValue(headers, "connection") != "keep-alive") {
			if (epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, &ev) == -1)
				cerr << "epoll_ctl failed" << endl;
			close(clientFd);
			if (position != monitor.end())
				monitor.erase(position);
		} else {
			ev.events = EPOLLIN;
			monitor[clientFd].fd = clientFd;
			ev.data.ptr = &monitor[clientFd];
			if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev) == -1) {
				cerr << "epoll_ctl failed" << endl;
				close(clientFd);
				if (position != monitor.end())
					monitor.erase(position);
			}
		}
		s.erase(s.find(clientFd));
	}
	else if (status == ss_cclosedcon) {
		if (epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, &ev) == -1)
			perror("epoll_ctl failed");
		close(clientFd);
		s.erase(s.find(clientFd));
		if (position != monitor.end())
			monitor.erase(position);
	}
}

static void	reqSessionStatus(const int& epollFd, const int& clientFd, map<int, httpSession>& s, const e_sstat& status) {
	struct epoll_event	ev;
	map<int, epollPtr>&	monitor = getEpollMonitor();
	map<int, epollPtr>::iterator	position = monitor.find(clientFd);

	if (status == ss_sHeader) {
		ev.events = EPOLLOUT;
		monitor[clientFd].fd = clientFd;
		ev.data.ptr = &monitor[clientFd];
		if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev) == -1) {
			cerr << "epoll_ctl failed" << endl;
			close(clientFd);
			s.erase(s.find(clientFd));
			if (position != monitor.end())
				monitor.erase(position);
		}
	}
	else if (status == ss_cclosedcon) {
		if (epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, &ev) == -1)
			cerr <<"epoll_ctl failed" << endl;
		close(clientFd);
		s.erase(s.find(clientFd));
		if (position != monitor.end())
			monitor.erase(position);
	}
}

static void	acceptNewClient(const int& epollFd, const int& serverFd) {
	struct epoll_event	ev;
	int					clientFd;
	map<int, epollPtr>&	monitor = getEpollMonitor();

	if ((clientFd = accept(serverFd, NULL, NULL)) < 0) {
		cerr << "accept failed" << endl;
		return;
    }
	monitor[clientFd].s = NULL;
	monitor[clientFd].fd = clientFd;
	ev.events = EPOLLIN;
	ev.data.ptr = &monitor[clientFd];
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &ev) == -1) {
		cerr << "epoll_ctl failed" << endl;
		map<int, epollPtr>::iterator	position = monitor.find(clientFd);
		close(clientFd);
		if (position != monitor.end())
			monitor.erase(position);
		return;
	}
	cerr << "new client added, its fd is -> " << clientFd << endl;
}


bool checkTimeOut(map<int, epollPtr>& monitor, const int& fd, epollPtr client, const int& epollFd) {
	struct epoll_event	ev;
	time_t lastActivityTime = client.timer;
    
	if ((lastActivityTime != 0 && time(NULL) - lastActivityTime >= T)) {
		cout << "Client " << fd << " TIMED OUT: " << time(NULL) - lastActivityTime << ".1" << endl;
		if (client.pid != -1) {
			kill(client.pid, 9);
		}
		if (epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, &ev) == -1) {
			cerr << "epoll_ctl failed" << endl;
		}
		close(fd);
		monitor.erase(monitor.find(fd));
		return false;
    }
	return true;
}

void checkTimeOutForEachUsr(const int& epollFd) {
	map<int, epollPtr>& monitor = getEpollMonitor();
	map<int, epollPtr>::iterator			it;

	for (it = monitor.begin(); it != monitor.end(); ++it) {
		if (it->second.is_server_socket == true)
			continue;
		if (checkTimeOut(monitor, it->first, it->second, epollFd) == false)
			it = monitor.begin();
	}
}


void	multiplexerSytm(const vector<int>& servrSocks, const int& epollFd, map<string, configuration>& config) {
	struct epoll_event					events[MAX_EVENTS];
	map<int, httpSession>				sessions;
	int									nfds;

	while (true) {
		if (shouldStop(0) == false)
			break;
		if ((nfds = epoll_wait(epollFd, events, MAX_EVENTS, 0)) == -1) {
			cerr << "epoll_wait failed" << endl;
        	if (errno == ENOMEM) {
				//ENOMEM is set when there'snt enough memory left in device
        	    for (map<int, httpSession>::iterator it = sessions.begin(); it != sessions.end(); ++it) {
					close(it->first);
				}
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
					acceptNewClient(epollFd, fd);
				else if (events[i].events & EPOLLIN) {
					if (ptr->s) {
						if (readCgiOutput(events[i])) {
							epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
							close(fd);
							ptr->s->setStatus();
						}
					} else {
						if (sessions.find(fd) == sessions.end()) {
							pair<int, httpSession> newclient(fd, httpSession(fd, config[ft_getsockname(fd)]));
							sessions.insert(newclient);
						}
						sessions[fd].req.readfromsock();
						reqSessionStatus(epollFd, fd, sessions, sessions[fd].status());
					}
				}
				else if (events[i].events & EPOLLOUT) {
					if (ptr->s) {
						if (writeBodyToCgi(events[i])) {
							epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
							close(fd);
							ptr->s->setStatus();
						}
					} else {
						sessions[fd].res.handelClientRes(epollFd);
						resSessionStatus(epollFd, fd, sessions, sessions[fd].status());
					}
				}
			}
			catch (const statusCodeException& exception) {
				errorResponse(epollFd, fd, sessions, exception);
			}
		}
		checkTimeOutForEachUsr(epollFd);
	}
	
}