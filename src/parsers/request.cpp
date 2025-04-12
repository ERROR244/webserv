#include "httpSession.hpp"

httpSession::Request::Request(httpSession& session) : s(session), length(0), fd(-1) {}

void	httpSession::Request::readfromsock() {
	char	buffer[BUFFER_SIZE];
	ssize_t byteread;
	ssize_t bufferPos = 0;
	if ((byteread = recv(s.clientFd, buffer, BUFFER_SIZE, MSG_DONTWAIT)) <= 0) {
		s.sstat = ss_cclosedcon;
		return ;
	}
	bstring bbuffer(buffer, byteread);
	bbuffer = remainingBody + bbuffer;
	remainingBody = NULL;//reseting to null be filled w new content in this iteration
	cerr << "raw buffer" << endl;
	cerr << bbuffer;
	cerr << "----------------" << endl;
	if (static_cast<int>(s.sstat) < 9) {
		bufferPos = parseStarterLine(bbuffer);
		if ((bufferPos = s.parseFields(bbuffer, bufferPos, s.headers)) < 0)
			throw(statusCodeException(400, "Bad Request14"));
		// checkNecessaryHeaders();
		if (s.sstat == ss_body)
			bodyFormat();
	}
	if (static_cast<size_t>(bufferPos) < bbuffer.size())
		(this->*bodyHandlerFunc)(bbuffer, bufferPos);
}