#include "httpSession.hpp"
#include "server.h"

httpSession::Response::Response(httpSession& session) : s(session), addChunkedWhenSendingCgiBody(false), cgiHeadersParsed(false) {}

httpSession::Response::Response(const Response& other) : s(other.s), cgiBuffer(other.cgiBuffer), addChunkedWhenSendingCgiBody(false), cgiHeadersParsed(false) {}

httpSession::Response::~Response() {
    inputFile.close();
}

void	httpSession::Response::handelClientRes(const int epollFd) {
	map<int, epollPtr>&	monitor = getEpollMonitor();
    if(s.cgi) {
		if (s.sstat == ss_sHeader) {
			struct epoll_event	evWritePipe;
			struct epoll_event	evReadPipe;

			//executing the cgi script
			s.cgi->prepearingCgiEnvVars(s.headers);
			s.cgi->setupCGIProcess();

			//loggin the script infos to the client who executed it so it can have conttol over it
			monitor[s.clientFd].cgiInfo.pid = s.cgi->ppid();

			//adding the pipes to epoll to be monitored too
			if (s.cgiBody.empty() == false) {
				monitor[s.clientFd].cgiInfo.writePipe = s.cgi->wFd();
				monitor[s.cgi->wFd()].fd = s.cgi->wFd();
				monitor[s.cgi->wFd()].s = &s;
				monitor[s.cgi->wFd()].type = cgiPipe;
				evWritePipe.events = EPOLLOUT;
				evWritePipe.data.ptr = &monitor[s.cgi->wFd()];
				if (epoll_ctl(epollFd, EPOLL_CTL_ADD, s.cgi->wFd(), &evWritePipe) == -1) {
					cerr << "epoll_ctl failed" << endl;
					s.sstat = ss_cclosedcon;
					return;
				}
			} else {
				close(s.cgi->wFd());
			}
			monitor[s.clientFd].cgiInfo.readPipe = s.cgi->rFd();
			monitor[s.cgi->rFd()].fd = s.cgi->rFd();
			monitor[s.cgi->rFd()].s = &s;
			monitor[s.cgi->rFd()].type = cgiPipe;
			evReadPipe.events = EPOLLIN;
			evReadPipe.data.ptr = &monitor[s.cgi->rFd()];
			if (epoll_ctl(epollFd, EPOLL_CTL_ADD, s.cgi->rFd(), &evReadPipe) == -1) {
				cerr << "epoll_ctl failed" << endl;
				s.sstat = ss_cclosedcon;
				return;
			}
			s.sstat = ss_CgiResponse;
		} else if (s.sstat == ss_CgiResponse)
			sendCgiOutput();
	} else {
		if (s.sstat == ss_sHeader) {
			sendHeader();
			monitor[s.clientFd].wroteInsock = true;
		}
		else if (s.sstat == ss_sBody) 
			sendBody();
		else if (s.sstat == ss_sBodyAutoindex) {
			generateHtml();
		}
		else {
			s.sstat = ss_done;
		}
	}
}

void	httpSession::Response::storeCgiResponse(const bstring& newBuff) {
	cgiBuffer += newBuff;
}

