#include "httpSession.hpp"

httpSession::Response::Response(httpSession& session) : headerSended(false), s(session), contentFd(-1), state(PROCESSING), lastActivityTime(0) {}

time_t	httpSession::Response::handelClientRes(const int clientFd) {
	if (s.cgi == NULL) {
		struct stat file_stat;

        if (stat(s.path.c_str(), &file_stat) == -1) {
			throw (statusCodeException(500, "Internal Server Error"));
		}
		else if (file_stat.st_size < BUFFER_SIZE) {
			sendRes(clientFd, true, file_stat);
		}
		else {
			sendRes(clientFd, false, file_stat);
		}
	} else {
		if (state == PROCESSING) {
			sendCgiStarterLine(clientFd);
			if (state == CCLOSEDCON)
				return 0;
			state = SHEADER;
			s.cgi->setupCGIProcess();
			cerr << "here" << endl;
		}
		sendCgiOutput(clientFd);
	}
    if (s.headers["connection"] != "keep-alive")
        return -1;
    return lastActivityTime;
}

void httpSession::Response::sendRes(int clientFd, bool smallFile, struct stat file_stat) {
    if (s.method == "GET") {
        if (headerSended == false) {
            GET(clientFd, smallFile);
        }
        else {
            sendBodyifChunked(clientFd);
        }
    }
    if (s.method == "POST") {
        cout << "POST method called\n";
        lastActivityTime = time(NULL);      // for timeout
        state = DONE;
    }
    if (s.method == "DELETE") {
        // string response;
        // response = "HTTP/1.1 204 No Content\r\nContent-Type: text/html\r\nConnection: keep-alive\r\n\r\n";
        // cout << "DELETE method called\n";
        // if (s.path.find("var/www/uploads") != 0) {
        //     std::cerr << "Directory Traversal Attempt While Deleting.\n";
        //     response = "HTTP/1.1 403 Forbidden\r\nContent-Type: text/html\r\ncontent-length: 0\r\nConnection: keep-alive\r\n\r\n";
        // }
        // else if (S_ISDIR(file_stat.st_mode) != 0) {
        //     if (remove(s.path.c_str()) != 0) {
        //         std::cerr << "Deleting Non-Empty Directory.\n";
        //         response = "HTTP/1.1 409 Conflict\r\nContent-Type: text/html\r\ncontent-length: 0\r\nConnection: keep-alive\r\n\r\n";
        //     }
        // }
        // else if (S_ISREG(file_stat.st_mode) != 0) {
        //     unlink(s.path.c_str());
        // }
        // cout << response << endl;
        // send(clientFd, response.c_str(), response.size(), MSG_DONTWAIT);
        // ev.events = EPOLLIN ;
        // ev.data.fd = clientFd;
        // epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev);
        lastActivityTime = time(NULL);      // for timeout
        state = DONE;
    }
}

const t_state&	httpSession::Response::status() const {
	return state;
}

void	httpSession::Response::setStatus() {
	state = PROCESSING;
}

void	checkTimeOut(map<int, time_t>& timeOut, const int& clientFd, time_t lastActivityTime) {
	if (lastActivityTime != 0 && time(NULL) - lastActivityTime >= T) {
        if (lastActivityTime == -1)
            cout << "Client " << clientFd << " connection closed." << endl;
        else
            cout << "Client " << clientFd << " TIMED OUT: " << time(NULL) - lastActivityTime << ".1" << endl;
        close(clientFd);
        timeOut.erase(timeOut.find(clientFd));
    }
}
