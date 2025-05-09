#include "httpSession.hpp"
#include "server.h"

bool	readCgiOutput(struct epoll_event ev) {
	epollPtr    *ptr = static_cast<epollPtr*>(ev.data.ptr);
	char        buffer[BUFFER_SIZE];
	ssize_t     bytesRead;

	if ((bytesRead = read(ptr->fd, buffer, BUFFER_SIZE)) <= 0) {
		cerr << "read failed" << endl;
		ptr->s->closeCon();
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
		cerr << "write failed" << endl;
		ptr->s->closeCon();
		return true;
	}
	body.erase(0, byteWrite);
	if (body.empty())
		return true;
	return false;
}

static bstring    tweakAndCheckHeaders(map<string, vector<string> >& headers) {
	bstring bheaders;

	if (headers.find("content-type") == headers.end())
		headers["content-type"].push_back("text/plain");
	if (headers.find("content-length") == headers.end() && headers.find("transfer-encoding") == headers.end())
		throw(statusCodeException(500, "Internal Server Error"));
	if (headers.find("content-length") != headers.end()) {
		try {
			my_stoi(headers["content-length"][0]);
		} catch (...) {
			throw(statusCodeException(500, "Internal Server Error"));
		}
	} else if (headers.find("transfer-encoding") != headers.end()) {
		if (headers["transfer-encoding"][0] != "chunked" && headers["transfer-encoding"][0] != "compress"
				&& headers["transfer-encoding"][0] != "deflate" && headers["transfer-encoding"][0] != "gzip")
		{
			throw(statusCodeException(500, "Internal Server Error"));
		}
	}
	for (map<string, vector<string> >::iterator it = headers.begin(); it != headers.end(); ++it) {
		if (it->first == "set-cookie") {
			for (size_t i = 0; i < it->second.size(); ++i)
				bheaders += (it->first + ": " + it->second[i] + "\r\n").c_str();
		} else {
			bheaders += (it->first + ": " + it->second[0] + "\r\n").c_str();
		}
	}
	bheaders += "\r\n";
	return bheaders;
}

void	closeCgiPipes(int epollFd, int readPipe, int writePipe) {
	epoll_ctl(epollFd, EPOLL_CTL_DEL, readPipe, NULL);
	epoll_ctl(epollFd, EPOLL_CTL_DEL, writePipe, NULL);
	close(readPipe);
	close(writePipe);
}


void    httpSession::Response::sendCgiOutput(const int epollFd) {
	int     			status;

	if (cgiBuffer.empty() == false) {
		bstring         chunkedResponse;
		ostringstream   chunkSize;
	
		if (cgiHeadersParsed == false) {
			map<int, epollPtr>& 			monitor = getEpollMonitor();
			map<string, vector<string> >	cgiHeaders;
			ssize_t  bodyStartPos = 0;

			s.sstat = ss_emptyline;
			try {
				if ((bodyStartPos = s.parseFields(cgiBuffer, 0, cgiHeaders)) < 0)
					return;
			} catch (...) {
				closeCgiPipes(epollFd, s.cgi->rFd(), s.cgi->wFd());
				throw(statusCodeException(500, "Internal Server Error"));
			}
			s.sstat = ss_CgiResponse;
			chunkedResponse += ("HTTP/1.1 " + toString(s.statusCode) + " " + s.codeMeaning + "\r\n").c_str();
			chunkedResponse += tweakAndCheckHeaders(cgiHeaders);
			cgiBuffer = cgiBuffer.substr(bodyStartPos);
			cgiHeadersParsed = true;
			monitor[s.clientFd].cgiInfo.responseSented = true;
		}
		chunkedResponse += cgiBuffer;
		if (send(s.clientFd, chunkedResponse.c_str(), chunkedResponse.size(), MSG_DONTWAIT | MSG_NOSIGNAL) <= 0) {
			cerr << "send failed" << endl;
			closeCgiPipes(epollFd, s.cgi->rFd(), s.cgi->wFd());
			s.sstat = ss_cclosedcon;
			return ;
		}
		cgiBuffer = NULL;
	} else if (waitpid(s.cgi->ppid(), &status, WNOHANG) > 0) {
		closeCgiPipes(epollFd, s.cgi->rFd(), s.cgi->wFd());
		s.sstat = ss_done;
	}
}