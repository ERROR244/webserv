#include "httpSession.hpp"
#include "server.h"

httpSession::Request::Request(httpSession& session) : s(session), length(0) {}

httpSession::Request::Request(const Request& other) : s(other.s), length(0) {}

httpSession::Request::~Request() {
	outputFile.close();
}

void	httpSession::Request::readfromsock() {
	char	buffer[BUFFER_SIZE];
	ssize_t byteread;
	ssize_t bufferPos = 0;

	if ((byteread = recv(s.clientFd, buffer, BUFFER_SIZE, MSG_DONTWAIT | MSG_NOSIGNAL)) <= 0) {
		s.sstat = ss_cclosedcon;
		return ;
	}
	bstring bbuffer(buffer, byteread);
	bbuffer = remainingBody + bbuffer;
	remainingBody = NULL;
	if (static_cast<int>(s.sstat) < 9) {
		map<int, epollPtr>&	monitor = getEpollMonitor();
		monitor[s.clientFd].timer = time(NULL);
		bufferPos = parseStarterLine(bbuffer);
		if ((bufferPos = s.parseFields(bbuffer, bufferPos, s.headers)) < 0)
			throw(statusCodeException(400, "Bad Request"));
		reconstructUri();
		if (s.method == POST)
			bodyFormat();
	}
	if (s.sstat == ss_body && static_cast<size_t>(bufferPos) < bbuffer.size())
		(this->*bodyHandlerFunc)(bbuffer, bufferPos);
}