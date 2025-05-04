#include "httpSession.hpp"
#include "server.h"

void	readCgiOutput(struct epoll_event ev) {
	epollPtr    *ptr = static_cast<epollPtr*>(ev.data.ptr);
	// httpSession *s = static_cast<httpSession*>(ptr->ptr);
	char        buffer[BUFFER_SIZE];
	ssize_t     bytesRead;

	if ((bytesRead = read(ptr->fd, buffer, BUFFER_SIZE)) <= 0) {
		//how can i show internall server error to the client incase of error
		cerr << "read failed" << endl;
	}
	bstring tmp(buffer, bytesRead);
	ptr->s->res.storeCgiResponse(tmp);
}

bool    writeBodyToCgi(struct epoll_event ev) {
	epollPtr	*ptr = static_cast<epollPtr*>(ev.data.ptr);
	// httpSession *s = static_cast<httpSession*>(ptr->ptr);
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
		headers["content-type"] = "application/octet-stream";
	if (headers.find("content-length") != headers.end())
		headers.erase("content-length");
	if (headers.find("transfer-encoding") != headers.end())
		headers.erase("transfer-encoding");
	headers["transfer-encoding"] = "chunked";
	if (headers.find("connection") == headers.end()) {
		headers["connection"] = "close";
	} else {
		if (headers["connection"] != "close" || headers["connection"] != "keep-alive")
			headers["connection"] = "close";
	}
	for (map<string, string>::iterator it = headers.begin(); it != headers.end(); ++it) {
		bheaders += (it->first + ": " + it->second + "\r\n").c_str();
	}
	bheaders += "\r\n";
	return bheaders;
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
				if ((bodyStartPos = s.parseFields(cgiBuffer, 0, cgiHeaders)) < 0) {
					//grahhhhh
					return;
				}
			} catch (...) {
				s.sstat = ss_cclosedcon;
				return;
			}
			s.sstat = ss_CgiResponse;
			chunkedResponse += ("HTTP/1.1 " + toString(s.statusCode) + " " + s.codeMeaning + "\r\n").c_str();
			chunkedResponse += tweakAndCheckHeaders(cgiHeaders);
			cgiBuffer = cgiBuffer.substr(bodyStartPos);
			cgiHeadersParsed = true;
		}
		chunkSize << hex << cgiBuffer.size() << "\r\n";
		chunkedResponse += chunkSize.str().c_str();
		chunkedResponse += cgiBuffer;
		chunkedResponse += "\r\n";
		if (send(s.clientFd, chunkedResponse.c_str(), chunkedResponse.size(), MSG_DONTWAIT | MSG_NOSIGNAL) <= 0) {
			cerr << "send failed" << endl;
			s.sstat = ss_cclosedcon;
			return ;
		}
		cgiBuffer = NULL;
	} else if (waitpid(s.cgi->ppid(), &status, WNOHANG) > 0) {
		struct epoll_event	ev;

		if (send(s.clientFd, "0\r\n\r\n", 5, MSG_DONTWAIT | MSG_NOSIGNAL) <= 0) {
			cerr << "send failed" << endl;
			s.sstat = ss_cclosedcon;
			return ;
		}
		epoll_ctl(epollFd, EPOLL_CTL_DEL, s.cgi->rFd(), &ev);
		epoll_ctl(epollFd, EPOLL_CTL_DEL, s.cgi->wFd(), &ev);
		close(s.cgi->rFd());
		close(s.cgi->wFd());
		s.sstat = ss_done;
	}
}