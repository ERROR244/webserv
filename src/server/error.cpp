#include "server.h"

static void	sendError(const int clientFd, const int statusCode, const string codeMeaning) {
	string msg;
	msg += "HTTP/1.1 " + to_string(statusCode) + " " + codeMeaning + "\r\n"; 
	msg += "Content-type: text/html\r\n";
	msg += "Transfer-Encoding: chunked\r\n";
	if (statusCode >= 500)
		msg += "Connection: close";
	else
		msg += "Connection: keep-alive\r\n";
	msg += "Server: bngn/0.1\r\n";
	msg += "\r\n";
	string body = "<!DOCTYPE html><html><body><h1>" + codeMeaning + "</h1></body></html>\r\n";
	ostringstream chunkSize;
	chunkSize << hex << body.size() << "\r\n";
	msg += chunkSize.str();
	msg += body;
	msg += "0\r\n\r\n";
	send(clientFd, msg.c_str(), msg.size(), MSG_DONTWAIT);
}

int	errorResponse(const int epollFd, const statusCodeException& exception, httpSession session) {
	cerr << "code--> " << exception.code() << endl;
	cerr << "reason--> " << exception.meaning() << endl;
	struct epoll_event	ev;
	configuration config = session.clientConfiguration();
	int	clientFd = session.fd();

	if (config.errorPages.find(exception.code()) != config.errorPages.end()) {
		session.reSetPath(w_realpath(("." + config.errorPages.at(exception.code())).c_str()));//use syscall realpath and not the wrapper
		ev.events = EPOLLOUT;
		ev.data.fd = clientFd;
		if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev) == -1) {
			perror("epoll_ctl faield");
			close(clientFd);
		}
	} else {
		sendError(clientFd, exception.code(), exception.meaning());
		if (exception.code() < 500) {
			ev.events = EPOLLIN;
			ev.data.fd = clientFd;
			if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev) == -1) {
				perror("epoll_ctl faield");
				close(clientFd);
			}
		} else {
			if (epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, &ev) == -1) {
				perror("epoll_ctl failed");
			}
			close(clientFd);
		}
		// delete session;
		return -1;
	}
	return 0;
}