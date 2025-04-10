#include "httpSession.hpp"

static bstring    tweakAndCheckHeaders(map<string, string>& headers) {
    bstring bheaders;

    if (headers.find("content-type") == headers.end())
        headers["content-type"] = "text/plain";
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

void    httpSession::Response::sendCgiOutput(const int clientFd) {
    char    buff[BUFFER_SIZE];
    int     byteRead = 0;
    int     status;

    if (!s.cgiBody.empty()) {
        int     byteWrite;

        cerr << s.cgi->wFd() << endl;
        if ((byteWrite = write(s.cgi->wFd(), s.cgiBody.c_str(), s.cgiBody.size())) <= 0) {
            perror("write failed(sendResponse.cpp 185)");
            s.sstat = ss_cclosedcon;
            return;
        }
        cerr << "byte write: " << byteWrite << endl;
        s.cgiBody.erase(0, byteWrite);
        if (s.cgiBody.empty())
            cerr << "done writing the vody to the script" << endl;
    }
    else if ((byteRead = read(s.cgi->rFd(), buff, BUFFER_SIZE)) < 0) {
        perror("read failed(sendResponse.cpp 152)");
        s.sstat = ss_cclosedcon;
        return;
    }
    bstring bbuffer(buff, byteRead);
    if (byteRead > 0) {
        bstring         chunkedResponse;
		ostringstream   chunkSize;
    
        if (cgiHeadersParsed == false) {
            map<string, string>	cgiHeaders;
            ssize_t  bodyStartPos = 0;

            s.sstat = ss_emptyline;
            try {
                if ((bodyStartPos = s.parseFields(bbuffer, 0, cgiHeaders)) < 0) { // i might use a buffer here in case of incomplete fields
                    perror("write failed(sendResponse.cpp 143)");
		            s.sstat = ss_cclosedcon;
                    return;
                }
            } catch (...) {
                s.sstat = ss_cclosedcon;
                return;
            }
            s.sstat = ss_sBody;
            chunkedResponse += ("HTTP/1.1 " + to_string(s.statusCode) + " " + s.codeMeaning + "\r\n").c_str();
            chunkedResponse += tweakAndCheckHeaders(cgiHeaders);
            bbuffer = bbuffer.substr(bodyStartPos);
            cgiHeadersParsed = true;
        }
		chunkSize << hex << bbuffer.size() << "\r\n";
        chunkedResponse += chunkSize.str().c_str();
        chunkedResponse += bbuffer;
        chunkedResponse += "\r\n";
        cerr << "cgi response ->>>>>>" << endl;
        cerr << chunkedResponse << endl;
        cerr << "-------------" << endl;
        if (send(clientFd, chunkedResponse.c_str(), chunkedResponse.size(), MSG_DONTWAIT) <= 0) {
            perror("send failed(sendResponse.cpp 50)");
			s.sstat = ss_cclosedcon;
			return ;
        }
    } else if (byteRead == 0 && waitpid(s.cgi->ppid(), &status, WNOHANG) > 0) {
        if (send(clientFd, "0\r\n\r\n", 5, MSG_DONTWAIT) <= 0) {
			perror("write failed(sendResponse.cpp 56)");
			s.sstat = ss_cclosedcon;
			return ;
		}
        s.sstat = ss_done;
		close(s.cgi->rFd());
		close(s.cgi->wFd());
    }
}