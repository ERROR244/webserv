#include "httpSession.hpp"

httpSession::Response::Response(httpSession& session) : s(session), contentFd(-1), cgiHeadersParsed(false) {}

httpSession::Response::~Response() {
    close(contentFd);
}

void	httpSession::Response::handelClientRes(const int clientFd) {
    if(s.cgi) {
		if (s.sstat == ss_sHeader) {
			s.cgi->prepearingCgiEnvVars(s.headers);
			s.cgi->setupCGIProcess();
			s.sstat = ss_sBody;
		}
		sendCgiOutput(clientFd);
        //incase of client closin the connection what will you do
		//send kill signal to the process if the client closed the connection
	} else {
		if (s.sstat == ss_sHeader) {
			sendHeader();
			if (s.sstat == ss_cclosedcon)
				return ;
		}
		if (s.sstat == ss_sBody)
			sendBody();
		else if (s.sstat == ss_sBodyAutoindex) {
			generateHtml();
		}
		else
			s.sstat = ss_done;
	}
}


// void httpSession::Response::sendRes(int clientFd, bool smallFile, struct stat& file_stat) {
//     if (s.method == GET) {
//         if (headerSended == false) {
//             Get(clientFd, smallFile);
//         }
//         else {
//             sendBodyifChunked(clientFd);
//         }
//     }
//     else if (s.method == POST) {
//         string response = "HTTP/1.1 204 No Content\r\n";
        
//         response += "content-length: 0\r\n";
//         response += "Connection: " + getConnection(s.getHeaders()["connection"]) + "\r\n\r\n";
//         send(clientFd, response.c_str(), response.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
//         s.sstat = ss_done;
//     }
//     else if (s.method == DELETE) {
//         string response = getDeleteRes(s.path, getConnection(s.getHeaders()["connection"]), file_stat);

//         send(clientFd, response.c_str(), response.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
//         s.sstat = ss_done;
//     }
// }