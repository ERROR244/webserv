#include "httpSession.hpp"

httpSession::Response::Response(httpSession& session) : s(session), contentFd(-1), state(PROCESSING) {}

void	httpSession::Response::sendResponse(const int clientFd) {
	if (s.cgi == NULL) {
		if (state == PROCESSING) {
			sendHeader(clientFd);
			if (state == CCLOSEDCON)
				return ;
			state = SHEADER;
		}
		if (s.method != "POST")
			sendBody(clientFd);
		else
			state = DONE;
	} else {
		if (state == PROCESSING) {
			sendCgiStarterLine(clientFd);
			if (state == CCLOSEDCON)
			return ;
			state = SHEADER;
			s.cgi->setupCGIProcess();
			cerr << "here" << endl;
		}
		sendCgiOutput(clientFd);
	}
}

const t_state&	httpSession::Response::status() const {
	return state;
}

