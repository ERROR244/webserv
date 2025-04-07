#include "httpSession.hpp"

httpSession::Response::Response(httpSession& session) : s(session), headerSended(false), contentFd(-1), cgiHeadersParsed(false), lastActivityTime(0) {}

void	httpSession::Response::handelRedirection(const int clientFd) {
    string response = "HTTP/1.1 301 Moved Permanently\n\r"
                        "Content-Length: " + toString(34 + s.returnedLocation.size()) + "\n\r"
                        "Content-Type: text/html; charset=utf-8\n\r"
                        "Location: " + s.returnedLocation + "\n\r\n\r"
                        "<a href='" + s.returnedLocation + "'>Moved Permanently</a>.\n\r";
    send(clientFd, response.c_str(), response.size(), MSG_DONTWAIT);
    s.sstat = done;
    lastActivityTime = time(NULL);
}

time_t	httpSession::Response::handelClientRes(const int clientFd) {
    if (!s.returnedLocation.empty()) {
        handelRedirection(clientFd);
    }
    else if(s.cgi) {
		if (s.sstat == sHeader) {
			s.cgi->prepearingCgiEnvVars(s.headers);
			s.cgi->setupCGIProcess();
			s.sstat = sBody;
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

string httpSession::Response::deleteFile(const string& file, const string& connection) {
    string response;

    unlink(file.c_str());
    response = "HTTP/1.1 204 No Content\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: 0\r\n";
    response += "Connection: " + (connection.empty() ? "close" : connection);
    response += "\r\n\r\n";
    return response;
}

string httpSession::Response::deleteDir(const string& dir, const string& connection) {
    string response = "";

    if (remove(dir.c_str()) != 0) {
        std::cerr << "Deleting Non-Empty Directory.\n";
        response += "HTTP/1.1 409 Conflict\r\n";
        response += "Content-Type: text/html\r\n";
        response += "content-length: 134\r\n";
        response += "Connection: " + (connection.empty() ? "close" : connection);
        response += "\r\n\r\n<!DOCTYPE html><html><head><title>409 Conflict</title></head><body><h1>Conflict</h1><p>Deleting Non-Empty Directory.</p></body></html>";
    }
    else {
        response += "HTTP/1.1 204 No Content\r\n";
        response += "Content-Type: text/html\r\n";
        response += "Content-Length: 0\r\n";
        response += "Connection: " + (connection.empty() ? "close" : connection);
        response += "\r\n\r\n";
    }
    return response;
}

string httpSession::Response::getDeleteRes(const string& path, const string& connection, struct stat& file_stat) {
    string response = "";

    if (path.find(s.rules->uploads) == string::npos) {
        std::cerr << "Directory Traversal Attempt While Deleting.\n";
        response += "HTTP/1.1 403 Forbidden\r\n";
        response += "Content-Type: text/html\r\n";
        response += "content-length: 139\r\n";
        response += "Connection: " + (connection.empty() ? "close" : connection);
        response += "\r\n\r\n<!DOCTYPE html><html><head><title>409 Conflict</title></head><body><h1>Conflict</h1><p>you don't have access to resource.</p></body></html>";
    }
    else if (S_ISDIR(file_stat.st_mode) != 0) {
        response = deleteDir(path, connection);
    }
    else if (S_ISREG(file_stat.st_mode) != 0) {
        response = deleteFile(path, connection);
    }
    else {
        response += "HTTP/1.1 403 Forbidden\r\n";
        response += "Content-Type: text/html\r\n";
        response += "content-length: 143\r\n";
        response += "Connection: " + (connection.empty() ? "close" : connection);
        response += "\r\n\r\n<!DOCTYPE html><html><head><title>403 Forbidden</title></head><body><h1>Forbidden</h1><p>resource is not a file or directory.</p></body></html>";
    }
    return response;
}

void httpSession::Response::sendRes(int clientFd, bool smallFile, struct stat& file_stat) {
    if (s.method == GET) {
        // cout << "GET method called on " << s.path << endl;
        if (headerSended == false) {
            Get(clientFd, smallFile);
        }
        else {
            sendBodyifChunked(clientFd);
        }
    }
    if (s.method == POST) {
        // cout << "POST method called on " << s.path << endl;
        string response;
        response += "HTTP/1.1 204 No Content\r\n";
        // response += "Content-Type: text/html\r\n";
        response += "content-length: 0\r\n";
        response += "Connection: " + (s.getHeaders()["connection"].empty() ? "close" : s.getHeaders()["connection"]) + "\r\n\r\n";
        send(clientFd, response.c_str(), response.size(), MSG_DONTWAIT);
        lastActivityTime = time(NULL);
        s.sstat = done;
    }
    if (s.method == DELETE) {
        string response = getDeleteRes(s.path, s.headers["connection"], file_stat);

        cout << "response---> " << response << endl;
        send(clientFd, response.c_str(), response.size(), MSG_DONTWAIT);
        lastActivityTime = time(NULL);
        s.sstat = done;
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