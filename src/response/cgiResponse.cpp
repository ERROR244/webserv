#include "httpSession.hpp"
#include "server.h"

void	readCgiOutput(epollPtr    *ptr) {
	char        buffer[BUFFER_SIZE];
	ssize_t     bytesRead;

	if ((bytesRead = read(ptr->fd, buffer, BUFFER_SIZE)) <= 0) {
		cerr << "read failed" << endl;
		ptr->s->closeCon();
	}
	bstring tmp(buffer, bytesRead);
	ptr->s->res.storeCgiResponse(tmp);
}

void    writeBodyToCgi(epollPtr    *ptr) {
	bstring& 	body = ptr->s->getCgiBody();
	int     	byteWrite;

	if (body.empty() == false) {
		if ((byteWrite = write(ptr->fd, body.c_str(), body.size())) <= 0) {
			cerr << "write failed" << endl;
			ptr->s->closeCon();
		}
		body.erase(0, byteWrite);
	}
}

static bstring    tweakAndCheckHeaders(map<string, vector<string> >& headers, bool& useChunked) {
	bstring bheaders;

	if (headers.find("content-type") == headers.end())
		headers["content-type"].push_back("text/plain");

	if (headers.find("content-length") == headers.end() && headers.find("transfer-encoding") == headers.end()) {
		//use transfer encoding
		headers["transfer-encoding"].push_back("chunked");
		useChunked = true;
	} else if (headers.find("content-length") != headers.end()) {
		try {
			my_stoi(headers["content-length"][0]);
		} catch (...) {
			//remove content-length and use transfer encoding
			headers.erase("content-length");
			headers.erase("transfer-encoding");
			headers["transfer-encoding"].push_back("chunked");
			useChunked = true;
		}
	} else if (headers.find("transfer-encoding") != headers.end()) {
		if (headers["transfer-encoding"][0] != "chunked" && headers["transfer-encoding"][0] != "compress"
				&& headers["transfer-encoding"][0] != "deflate" && headers["transfer-encoding"][0] != "gzip")
		{
			headers.erase("transfer-encoding");
			headers["transfer-encoding"].push_back("chunked");
			useChunked = true;
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

void    httpSession::Response::sendCgiOutput() {
	int     status;

	if (cgiBuffer.empty() == false) {
		bstring         cgiResponse;
	
		if (cgiHeadersParsed == false) {
			map<string, vector<string> >	cgiHeaders;
			ssize_t							bodyStartPos = 0;
			map<int, epollPtr>&				monitor = getEpollMonitor();

			s.sstat = ss_emptyline;
			try {
				if ((bodyStartPos = s.parseFields(cgiBuffer, 0, cgiHeaders)) < 0)
					return;
			} catch (...) {
				throw(statusCodeException(500, "Internal Server Error"));
			}
			s.sstat = ss_CgiResponse;
			cgiResponse += ("HTTP/1.1 " + toString(s.statusCode) + " " + s.codeMeaning + "\r\n").c_str();
			cgiResponse += tweakAndCheckHeaders(cgiHeaders, addChunkedWhenSendingCgiBody);
			cgiBuffer = cgiBuffer.substr(bodyStartPos);
			cgiHeadersParsed = true;
			if (send(s.clientFd, cgiResponse.c_str(), cgiResponse.size(), MSG_DONTWAIT | MSG_NOSIGNAL) <= 0) {
				cerr << "send failed" << endl;
				s.sstat = ss_cclosedcon;
				return ;
			}
			monitor[s.clientFd].wroteInsock = true;
		} else {
			if (addChunkedWhenSendingCgiBody) {
				ostringstream   chunkSize;
				chunkSize << hex << cgiBuffer.size() << "\r\n";
				cgiResponse += chunkSize.str().c_str();
			}
			cgiResponse += cgiBuffer;
			if (addChunkedWhenSendingCgiBody)
				cgiResponse += "\r\n";
			if (send(s.clientFd, cgiResponse.c_str(), cgiResponse.size(), MSG_DONTWAIT | MSG_NOSIGNAL) <= 0) {
				cerr << "send failed" << endl;
				s.sstat = ss_cclosedcon;
				return ;
			}
			cgiBuffer = NULL;
		}
	} else if (waitpid(s.cgi->ppid(), &status, WNOHANG) > 0) {
		if (addChunkedWhenSendingCgiBody) {
			if (send(s.clientFd, "0\r\n\r\n", 5, MSG_DONTWAIT | MSG_NOSIGNAL) <= 0) {
				cerr << "send failed" << endl;
				s.sstat = ss_cclosedcon;
				return ;
			}
		}
		s.sstat = ss_done;
	}
}