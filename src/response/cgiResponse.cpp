#include "httpSession.hpp"
#include "server.h"

bool	readCgiOutput(struct epoll_event ev) {
	epollPtr    *ptr = static_cast<epollPtr*>(ev.data.ptr);
	char        buffer[BUFFER_SIZE];
	ssize_t     bytesRead;

	if ((bytesRead = read(ptr->fd, buffer, BUFFER_SIZE)) <= 0) {
		//how can i show internall server error to the client incase of error
		cerr << "read failed" << endl;
		return true;
	}
	bstring tmp(buffer, bytesRead);
	ptr->s->res.storeCgiResponse(tmp);
	return false;
}

bool    writeBodyToCgi(struct epoll_event ev) {
	epollPtr	*ptr = static_cast<epollPtr*>(ev.data.ptr);
	bstring& 	body = ptr->s->getCgiBody();
	int     	byteWrite;

	if ((byteWrite = write(ptr->fd, body.c_str(), body.size())) <= 0) {
		//how can i show internall server error to the client incase of error
		cerr << "write failed" << endl;
		return true;
	}
	body.erase(0, byteWrite);
	if (body.empty())
		return true;
	return false;
}

static bstring    tweakAndCheckHeaders(map<string, string>& headers) {
	bstring bheaders;

	if (headers.find("content-type") == headers.end())
		headers["content-type"] = "text/plain";
	if (headers.find("connection") == headers.end()) {
		headers["connection"] = "close";
	} else {
		if (headers["connection"] != "close" && headers["connection"] != "keep-alive")
			headers["connection"] = "close";
	}
	for (map<string, string>::iterator it = headers.begin(); it != headers.end(); ++it) {
		bheaders += (it->first + ": " + it->second + "\r\n").c_str();
	}
	bheaders += "\r\n";
	return bheaders;
}

void	closecgiPipes(int epollFd, int readPipe, int writePipe) {
	epoll_ctl(epollFd, EPOLL_CTL_DEL, readPipe, NULL);
	epoll_ctl(epollFd, EPOLL_CTL_DEL, writePipe, NULL);
	close(readPipe);
	close(writePipe);
}


void    httpSession::Response::sendCgiOutput(const int epollFd) {
	int     status;

	if (cgiBuffer.empty() == false) {
		bstring         chunkedResponse;
		ostringstream   chunkSize;
	
		if (cgiHeadersParsed == false) {
			map<string, string>	cgiHeaders;
			ssize_t  bodyStartPos = 0;

			s.sstat = ss_emptyline;
			try {
				if ((bodyStartPos = s.parseFields(cgiBuffer, 0, cgiHeaders)) < 0)
					return;
			} catch (...) {
				closecgiPipes(epollFd, s.cgi->rFd(), s.cgi->wFd());
				throw(statusCodeException(500, "Internal Server Error"));
			}
			s.sstat = ss_CgiResponse;
			chunkedResponse += ("HTTP/1.1 " + toString(s.statusCode) + " " + s.codeMeaning + "\r\n").c_str();
			chunkedResponse += tweakAndCheckHeaders(cgiHeaders);
			cgiBuffer = cgiBuffer.substr(bodyStartPos);
			cgiHeadersParsed = true;
		}
		chunkedResponse += cgiBuffer;
		if (send(s.clientFd, chunkedResponse.c_str(), chunkedResponse.size(), MSG_DONTWAIT | MSG_NOSIGNAL) <= 0) {
			cerr << "send failed" << endl;
			closecgiPipes(epollFd, s.cgi->rFd(), s.cgi->wFd());
			s.sstat = ss_cclosedcon;
			return ;
		}
		cgiBuffer = NULL;
	} else if (waitpid(s.cgi->ppid(), &status, WNOHANG) > 0) {
		closecgiPipes(epollFd, s.cgi->rFd(), s.cgi->wFd());
		s.sstat = ss_done;
	}
}