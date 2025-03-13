#include "httpSession.hpp"

httpSession::Request::Request(httpSession& session) : s(session), requestStat(e_requestStat::headers), length(0), fd(-1) {}

void	httpSession::Request::readfromsock() {
	char	buffer[BUFFER_SIZE];
	ssize_t byteread;
	ssize_t bufferPos = 0;
	if ((byteread = recv(s.clientFd, buffer, BUFFER_SIZE, MSG_DONTWAIT)) <= 0) {
		s.sstat = cclosedcon;
		return ;
	}
	bstring bbuffer(buffer, byteread);
	bbuffer = remainingBody + bbuffer;
	remainingBody = NULL;//reseting to null be filled w new content in this iteration
	// cerr << "raw buffer" << endl;
	// cerr << bbuffer << endl;
	// cerr << "---" << endl;
	// switch (requestStat)
	// {
	// case e_requestStat::headers: {
	// 	bufferPos = parseStarterLine(bbuffer);
	// 	if ((bufferPos = s.parseFields(bbuffer, bufferPos, s.headers)) < 0)
	// 		throw(statusCodeException(400, "Bad Request"));
	// 	if (s.sstat == e_sstat::sHeader)
	// 		break;
	// 	requestStat = e_requestStat::bodyFormat;
	// 	break;
	// }
	// case e_requestStat::bodyFormat: {
	// 	bodyFormat();
	// 	requestStat = e_requestStat::handleBody;
	// }
	// case e_requestStat::handleBody: {
	// 	if (static_cast<size_t>(bufferPos) < bbuffer.size())
	// 		(this->*bodyHandlerFunc)(bbuffer, bufferPos);
	// }
	// }
	if (static_cast<int>(requestStat) < 9) {
		bufferPos = parseStarterLine(bbuffer);
		if ((bufferPos = s.parseFields(bbuffer, bufferPos, s.headers)) < 0)
			throw(statusCodeException(400, "Bad Request"));
		if (s.sstat == e_sstat::body) {
			bodyFormat();
			if (static_cast<size_t>(bufferPos) < bbuffer.size())
				(this->*bodyHandlerFunc)(bbuffer, bufferPos);
		} else {
			return ;
		}
	}
	else if (static_cast<size_t>(bufferPos) < bbuffer.size())
		(this->*bodyHandlerFunc)(bbuffer, bufferPos);
}