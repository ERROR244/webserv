#include "server.hpp"
#include "wrapperFunc.hpp"

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

void handelNewConnection(int eventFd, int epollFds) {
    int clientFd;
	struct epoll_event  ev;

    if ((clientFd = accept(eventFd, NULL, NULL)) == -1) {
        cerr << "Accept failed" << endl;
        return ;
    }
    cout << "\n--------------------------------------New client connected!--------------------------------------\n" << endl;

    // set the client socket to non-blocking mode
    if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0) {
        cerr << "Failed to set non-blocking" << endl;
        ft_close(clientFd, "clientFd");
        return ;
    }

    // add the new client socket to epoll
    ev.events = EPOLLIN;
    ev.data.fd = clientFd;
    if (epoll_ctl(epollFds, EPOLL_CTL_ADD, clientFd, &ev) == -1) {
        cerr << "epoll_ctl failed for client socket" << clientFd << endl;
        ft_close(clientFd, "clientFd");
        return ;
    }
    indexMap[clientFd].KV = confi.kValue[getsockname(clientFd)];
    indexMap[clientFd].headerSended = false;
    indexMap[clientFd].clientFd = clientFd;
    indexMap[clientFd].fileFd = -1;
    indexMap[clientFd].lastRes = 0;
}

string toString(const int& nbr) {
    ostringstream oss;

    oss << nbr;
    return (oss.str());
}

string getsockname(int clientFd) {
    struct sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    string res;

    if (getsockname(clientFd, (struct sockaddr*)&addr, &addrLen) == 0) {
        res = string(inet_ntoa(addr.sin_addr)) + ":" + toString(ntohs(addr.sin_port));
        return res;
    } else {
        throw "getsockname failed";
    }
}

void	multiplexerSytm(vector<int>& servrSocks, const int& epollFd, map<string, configuration>& config) {
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
			int fd = events[i].data.fd;
			try {
				// if (find(servrSocks.begin(), servrSocks.end(), fd) != servrSocks.end() && indexMap[fd].lastRes != 0 && time(nullptr) - indexMap[fd].lastRes > T) {
				// 	cout << indexMap[fd].lastRes << endl;
				// 	if (fd >= 0)
				// 		ft_close(fd, "clientFd");
				// }
				if (find(servrSocks.begin(), servrSocks.end(), fd) != servrSocks.end()) {
					string clientClass = getsockname(fd);
					sessions.try_emplace(fd, new httpSession(fd, &(config[clientClass])));
					handelNewConnection(epollFd, fd);
				}
				else if (events[i].events & EPOLLIN) {
					sessions[fd]->req.parseMessage(fd);
					sessions[fd]->clientClass = clientClass;
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
				// if (config.errorPages.find(exception.code()) != config.errorPages.end()) {
				// 	sessions[fd]->reSetPath(w_realpath(("." + config.errorPages.at(exception.code())).c_str()));
				// 	ev.events = EPOLLOUT;
				// 	ev.data.fd = fd;
				// 	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev) == -1) {
				// 		perror("epoll_ctl faield(setUpserver.cpp): "); exit(-1);
				// 	}
				// } else {
				// 	sendError(fd, exception.code(), exception.meaning());
				// 	ev.events = EPOLLIN;
				// 	ev.data.fd = fd;
				// 	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev) == -1) {
				// 		perror("epoll_ctl faield(setUpserver.cpp): "); exit(-1);
				// 	}
				// 	sessions.erase(sessions.find(fd));
				// }
			}
		}
	}
}