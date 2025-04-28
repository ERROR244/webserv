#include "httpSession.hpp"

httpSession::Response::Response(httpSession& session) : s(session), cgiHeadersParsed(false) {}

httpSession::Response::~Response() {
    inputFile.close();
}

void	httpSession::Response::handelClientRes(const int clientFd) {
    if(s.cgi) {
		if (s.sstat == ss_sHeader) {
			s.cgi->prepearingCgiEnvVars(s.headers);
			s.cgi->setupCGIProcess();
			s.sstat = ss_sBody;
		}
		sendCgiOutput(clientFd);
        //incase of client closin the connection what will you do
		//send kill signal to the process if the client closed the connection
	} else {
		if (s.sstat == ss_sHeader)
			sendHeader();
		else if (s.sstat == ss_sBody)
			sendBody();
		else if (s.sstat == ss_sBodyAutoindex) {
			generateHtml();
		}
		else
			s.sstat = ss_done;
	}
}