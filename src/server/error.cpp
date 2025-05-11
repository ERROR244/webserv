#include "server.h"
#include "httpSession.hpp"


static void	sendError(const int clientFd, const int statusCode, const string codeMeaning) {
	string			msg;
	string			body;

	msg += "HTTP/1.1 " + toString(statusCode) + " " + codeMeaning + "\r\n"; 
	msg += "Content-type: text/html\r\n";
	msg += "Connection: close\r\n";
	msg += "Server: bngn/0.1\r\n";
	body = "<!DOCTYPE html><html><body><h1>" + codeMeaning + "</h1></body></html>\r\n";
	msg += "content-length: " + toString(body.size()) + "\r\n\r\n";
	msg += body;
	send(clientFd, msg.c_str(), msg.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
}

void	errorResponse(const int epollFd, int clientFd, map<int, httpSession>& sessions, const statusCodeException& exception) {
	struct epoll_event				ev;
	bool							isCgi;
	map<int, epollPtr>&				monitor = getEpollMonitor();
	configuration 					config = sessions[clientFd].clientConfiguration();

	if (monitor[clientFd].cgiInfo.pid != -1)
		kill(monitor[clientFd].cgiInfo.pid, 9);
	cleanCgiRessources(epollFd, clientFd, isCgi);
	if (config.errorPages.find(exception.code()) != config.errorPages.end()) {
		sessions[clientFd].resetForSendingErrorPage(config.errorPages.at(exception.code()), exception.code(), exception.meaning());
		ev.events = EPOLLOUT;
		ev.data.ptr = &monitor[clientFd];
		if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev) == -1) {
			perror("epoll_ctl failed");
			sessions.erase(clientFd);
			monitor.erase(clientFd);
			close(clientFd);
		}
	} else {
		sendError(clientFd, exception.code(), exception.meaning());
		if (epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, &ev) == -1)
			perror("epoll_ctl failed");
		sessions.erase(clientFd);
		monitor.erase(clientFd);
		close(clientFd);
	}
}