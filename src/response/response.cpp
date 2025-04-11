#include "httpSession.hpp"

httpSession::Response::Response(httpSession& session) : s(session), headerSended(false), contentFd(-1), cgiHeadersParsed(false), lastActivityTime(0) {}

string getConnection(string ConHeadre) {
    string Connection;

    if (ConHeadre.empty() || ConHeadre == "keep-alive") {
        Connection = "keep-alive";
    }
    else {
        Connection = "close";
    }
    return Connection;
}

void	httpSession::Response::handelRedirection(const int clientFd) {
    std::string body = "<a href='" + s.returnedLocation + "'>Moved Permanently</a>.\n";
    std::string response = "HTTP/1.1 301 Moved Permanently\r\n"
                        "Content-Length: " + toString(body.length()) + "\r\n"
                        "Content-Type: text/html; charset=utf-8\r\n"
                        "Location: " + s.returnedLocation + "\r\n"
                        "Connection: " + getConnection(s.getHeaders()["connection"]) + "\r\n"
                        "\r\n" + body;
    
    send(clientFd, response.c_str(), response.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
    s.sstat = ss_done;
    lastActivityTime = time(NULL);
}

time_t	httpSession::Response::handelClientRes(const int clientFd) {
    if (!s.returnedLocation.empty()) {
        handelRedirection(clientFd);
    }
    else if(s.cgi) {
		if (s.sstat == ss_sHeader) {
			s.cgi->prepearingCgiEnvVars(s.headers);
			s.cgi->setupCGIProcess();
			s.sstat = ss_sBody;
		}
		sendCgiOutput(clientFd);
        //incase of client closin the connection what will you do
	} else {
		struct stat file_stat;

        if (stat(s.path.c_str(), &file_stat) == -1)
			throw (statusCodeException(500, "Internal Server Error (stat)"));
		else if (file_stat.st_size < BUFFER_SIZE)
			sendRes(clientFd, true, file_stat);
		else
			sendRes(clientFd, false, file_stat);
	}
    if (s.headers["connection"] != "keep-alive")
        return -1;
    return lastActivityTime;
}


void httpSession::Response::sendRes(int clientFd, bool smallFile, struct stat& file_stat) {
    if (s.method == GET) {
        if (headerSended == false) {
            Get(clientFd, smallFile);
        }
        else {
            sendBodyifChunked(clientFd);
        }
    }
    else if (s.method == POST) {
        string response = "HTTP/1.1 204 No Content\r\n";
        
        response += "content-length: 0\r\n";
        response += "Connection: " + getConnection(s.getHeaders()["connection"]) + "\r\n\r\n";
        send(clientFd, response.c_str(), response.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
        lastActivityTime = time(NULL);
        s.sstat = ss_done;
    }
    else if (s.method == DELETE) {
        string response = getDeleteRes(s.path, getConnection(s.getHeaders()["connection"]), file_stat);

        send(clientFd, response.c_str(), response.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
        lastActivityTime = time(NULL);
        s.sstat = ss_done;
    }
}

bool checkTimeOut(map<int, time_t>& timeOut, const int& clientFd, time_t lastActivityTime) {
	bool timedOut = false;

    if ((lastActivityTime != 0 && time(NULL) - lastActivityTime >= T)) {
        if (lastActivityTime == -1) {
            cout << "Client " << clientFd << " connection closed." << endl;
        }
        else {
            cout << "Client " << clientFd << " TIMED OUT: " << time(NULL) - lastActivityTime << ".1" << endl;
        }
        timedOut = true;
        close(clientFd);
        timeOut.erase(timeOut.find(clientFd));
    }
    return timedOut;
}