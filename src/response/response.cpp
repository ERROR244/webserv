#include "httpSession.hpp"

httpSession::Response::Response(httpSession& session) : s(session), contentFd(-1), state(PROCESSING) {}

void httpSession::Response::sendRes(int clientFd, bool smallFile, struct stat file_stat) {
    if (indexMap[clientFd].method == "GET") {
        if (indexMap[clientFd].headerSended == false) {
            GET(clientFd, smallFile);
        }
        else {
            sendBodyifChunked(clientFd);
        }
    }
    if (indexMap[clientFd].method == "POST") {
        cout << "POST method called\n";
    }
    if (indexMap[clientFd].method == "DELETE") {
        string response;
        response = "HTTP/1.1 204 No Content\r\nContent-Type: text/html\r\nConnection: keep-alive\r\n\r\n";
        cout << "DELETE method called\n";
        if (indexMap[clientFd].requestedFile.find("var/www/uploads") != 0) {
            std::cerr << "Directory Traversal Attempt While Deleting.\n";
            response = "HTTP/1.1 403 Forbidden\r\nContent-Type: text/html\r\ncontent-length: 0\r\nConnection: keep-alive\r\n\r\n";
        }
        else if (S_ISDIR(file_stat.st_mode) != 0) {
            if (remove(indexMap[clientFd].requestedFile.c_str()) != 0) {
                std::cerr << "Deleting Non-Empty Directory.\n";
                response = "HTTP/1.1 409 Conflict\r\nContent-Type: text/html\r\ncontent-length: 0\r\nConnection: keep-alive\r\n\r\n";
            }
        }
        else if (S_ISREG(file_stat.st_mode) != 0) {
            unlink(indexMap[clientFd].requestedFile.c_str());
        }
        cout << response << endl;
        send(clientFd, response.c_str(), response.size(), MSG_DONTWAIT);
        ev.events = EPOLLIN ;
        ev.data.fd = clientFd;
        epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev);
        indexMap[clientFd].lastRes = time(nullptr);
    }
}

void	httpSession::Response::handelClientRes(const int clientFd) {
	if (s.cgi == NULL) {
		struct stat file_stat;
    
		if (stat(indexMap[clientFd].requestedFile.c_str(), &file_stat) == -1) {
			throw(statusCodeException(500, "Internal Server Error"));
		}
		else if (file_stat.st_size < 10000) {
			sendRes(clientFd, true, file_stat);
		}
		else {
			sendRes(clientFd, false, file_stat);
		}

		// if (state == PROCESSING) {
		// 	sendHeader(clientFd);
		// 	if (state == CCLOSEDCON)
		// 		return ;
		// 	state = SHEADER;
		// }
		// if (s.method != "POST")
		// 	sendBody(clientFd);
		// else
		// 	state = DONE;
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