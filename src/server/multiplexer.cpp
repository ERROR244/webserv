#include "server.h"

void	cleanCgiRessources(int epollFd, int clientFd, bool& isCgi) {
	map<int, epollPtr>&				monitor = getEpollMonitor();

	if (monitor[clientFd].cgiInfo.pid != -1) {
		cerr << "freing cgi ressources" << endl;
		monitor[clientFd].cgiInfo.pid = -1;
		int& readPipe = monitor[clientFd].cgiInfo.readPipe;
		int& writePipe = monitor[clientFd].cgiInfo.writePipe;

		if (readPipe != -1) {
			if (epoll_ctl(epollFd, EPOLL_CTL_DEL, readPipe, NULL) == -1)
				cerr << "epoll_ctl failed" << endl;
			monitor.erase(readPipe);
		}
		if (writePipe != -1) {
			if (epoll_ctl(epollFd, EPOLL_CTL_DEL, writePipe, NULL) == -1)
				cerr << "epoll_ctl failed" << endl;
			monitor.erase(writePipe);
		}
		close(readPipe);
		close(writePipe);
		monitor[clientFd].cgiInfo.pid = -1;
		readPipe = -1;
		writePipe = -1;
		isCgi = true;
	}
}

static bool	resSessionStatus(const int& epollFd, const int& clientFd, map<int, httpSession>& s, const e_sstat& status) {
	struct epoll_event				ev;
	map<int, epollPtr>&				monitor = getEpollMonitor();
	bool							isCgi = false;

	if (status == ss_done) {
		cleanCgiRessources(epollFd, clientFd, isCgi);
		map<string, vector<string> > headers = s[clientFd].getHeaders();
		if (headers.find("connection") == headers.end() || getHeaderValue(headers, "connection") != "keep-alive") {
			if (epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, NULL) == -1)
				cerr << "epoll_ctl failed" << endl;
			monitor.erase(clientFd);
			close(clientFd);
		} else {
			ev.events = EPOLLIN;
			monitor[clientFd].timer = 0;
			ev.data.ptr = &monitor[clientFd];
			if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev) == -1) {
				cerr << "epoll_ctl failed" << endl;
				monitor.erase(clientFd);
				close(clientFd);
			}
		}
		s.erase(clientFd);
	}
	else if (status == ss_cclosedcon) {
		if (monitor[clientFd].cgiInfo.pid != -1)
			kill(monitor[clientFd].cgiInfo.pid, 9);
		cleanCgiRessources(epollFd, clientFd, isCgi);
		if (epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, NULL) == -1)
			perror("epoll_ctl failed");
		monitor.erase(clientFd);
		s.erase(clientFd);
		close(clientFd);
	}
	return isCgi;
}

static void	reqSessionStatus(const int& epollFd, const int& clientFd, map<int, httpSession>& s, const e_sstat& status) {
	struct epoll_event	ev;
	map<int, epollPtr>&	monitor = getEpollMonitor();

	if (status == ss_sHeader) {
		ev.events = EPOLLOUT;
		ev.data.ptr = &monitor[clientFd];
		if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev) == -1) {
			cerr << "epoll_ctl failed" << endl;
			s.erase(clientFd);
			monitor.erase(clientFd);
			close(clientFd);
		}
	}
	else if (status == ss_cclosedcon) {
		if (epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, &ev) == -1)
			cerr <<"epoll_ctl failed" << endl;
		s.erase(clientFd);
		monitor.erase(clientFd);
		close(clientFd);
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
	monitor[clientFd].fd = clientFd;
	monitor[clientFd].type = clientSock;
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

void checkTimeOutForEachUsr(const int& epollFd, map<int, httpSession>& sessions) {
	map<int, epollPtr>& monitor = getEpollMonitor();
	map<int, epollPtr>::iterator			it;

	for (it = monitor.begin(); it != monitor.end(); ++it) {
		if (it->second.type == serverSock || it->second.type == cgiPipe) {
			continue;
		}
		if (it->second.timer != 0 && time(NULL) - it->second.timer >= T) {
			bool				isCgi;

			cout << "Client " << it->first << " TIMED OUT" << endl;
			if (it->second.wroteInsock == false)
				errorResponse(epollFd, it->first, sessions, statusCodeException(408, "Timeout"));
			else {
				if (it->second.cgiInfo.pid != -1)
					kill(it->second.cgiInfo.pid, 9);
				cleanCgiRessources(epollFd, it->first, isCgi);
				if (epoll_ctl(epollFd, EPOLL_CTL_DEL, it->first, NULL) == -1)
					perror("epoll_ctl failed");
				close(it->first);
				sessions.erase(it->first);
				monitor.erase(it->first);
			}
			return;
		}
	}
}


void	multiplexerSytm(const int& epollFd, map<string, configuration>& config) {
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
				if (ptr->type == serverSock)
					acceptNewClient(epollFd, fd);
				else if (events[i].events & EPOLLIN) {
					if (ptr->type == cgiPipe) {
						readCgiOutput(ptr);
					} else if (ptr->type == clientSock) {
						if (sessions.find(fd) == sessions.end()) {
							pair<int, httpSession> newclient(fd, httpSession(fd, config[ft_getsockname(fd)]));
							sessions.insert(newclient);
						}
						sessions[fd].req.readfromsock();
						reqSessionStatus(epollFd, fd, sessions, sessions[fd].status());
					}
				} else if (events[i].events & EPOLLOUT) {
					if (ptr->type == cgiPipe) {
						writeBodyToCgi(ptr);
					} else if (ptr->type == clientSock) {
						sessions[fd].res.handelClientRes(epollFd);
						if (resSessionStatus(epollFd, fd, sessions, sessions[fd].status()) == true)
							break;
					}
				}
			}
			catch (const statusCodeException& exception) {
				errorResponse(epollFd, fd, sessions, exception);
			}
		}
		checkTimeOutForEachUsr(epollFd, sessions);
	}
}