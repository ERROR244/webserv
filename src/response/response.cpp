#include "httpSession.hpp"
#include "server.h"
httpSession::Response::Response(httpSession& session) : s(session), cgiHeadersParsed(false) {}

httpSession::Response::~Response() {
    inputFile.close();
}

void	httpSession::Response::handelClientRes(const int epollFd) {
    if(s.cgi) {
		if (s.sstat == ss_sHeader) {
			struct epoll_event	evWritePipe;
			struct epoll_event	evReadPipe;
			epollPtr			*tmp1 = new epollPtr;
			epollPtr			*tmp2 = new epollPtr;

			s.cgi->prepearingCgiEnvVars(s.headers);
			s.cgi->setupCGIProcess();
			if (s.cgiBody.empty() == false) {
				tmp1->fd = s.cgi->wFd();
				tmp1->ptr = &s;
				evWritePipe.events = EPOLLOUT;
				evWritePipe.data.ptr = tmp1;
				if (epoll_ctl(epollFd, EPOLL_CTL_ADD, s.cgi->wFd(), &evWritePipe) == -1) {
					cerr << "epoll_ctl failed" << endl;
					s.sstat = ss_cclosedcon;
					return;
				}
			}
			tmp2->fd = s.cgi->rFd();
			tmp2->ptr = &s;
			evReadPipe.events = EPOLLIN;
			evReadPipe.data.ptr = tmp2;
			if (epoll_ctl(epollFd, EPOLL_CTL_ADD, s.cgi->rFd(), &evReadPipe) == -1) {
				cerr << "epoll_ctl failed" << endl;
				s.sstat = ss_cclosedcon;
				return;
			}
			s.sstat = ss_CgiResponse;
		} else if (s.sstat == ss_CgiResponse) 
			sendCgiOutput(epollFd);
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

void	httpSession::Response::storeCgiResponse(const bstring& newBuff) {
	cgiBuffer += newBuff;
}