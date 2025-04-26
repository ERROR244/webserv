#include "server.h"
#include "httpSession.hpp"


static void	sendError(const int clientFd, const int statusCode, const string codeMeaning) {
	ostringstream	chunkSize;
	string			msg;
	string			body;

	msg += "HTTP/1.1 " + toString(statusCode) + " " + codeMeaning + "\r\n"; 
	msg += "Content-type: text/html\r\n";
	msg += "Connection: close\r\n";
	msg += "Server: bngn/0.1\r\n";
	body = "<!DOCTYPE html><html><body><h1>" + codeMeaning + "</h1></body></html>\r\n";
	msg += "content-length: " + toString(body.size()) + "\r\n\r\n";
	msg += body;
	send(clientFd, msg.c_str(), msg.size(), MSG_DONTWAIT);
}

void	errorResponse(const int epollFd, int clientFd, map<int, httpSession>& sessions, const statusCodeException& exception) {
	cerr << "code--> " << exception.code() << endl;
	cerr << "reason--> " << exception.meaning() << endl;
	struct epoll_event	ev;
	configuration config = sessions[clientFd].clientConfiguration();

	if (config.errorPages.find(exception.code()) != config.errorPages.end()) {
		sessions[clientFd].resetForSendingErrorPage(config.errorPages.at(exception.code()));
		ev.events = EPOLLOUT;
		ev.data.fd = clientFd;
		ft_epoll__ctl(epollFd, clientFd, &ev);
	} else {
		sendError(clientFd, exception.code(), exception.meaning());
		if (exception.code() < 500) {
			ev.events = EPOLLIN;
			ev.data.fd = clientFd;
			if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev) == -1) {
				perror("epoll_ctl failed");
				close(clientFd);
			}
		} else {
			if (epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, &ev) == -1) {
				perror("epoll_ctl failed");
			}
			close(clientFd);
		}
		sessions.erase(sessions.find(clientFd));
	}
}