#include "httpSession.hpp"

string httpSession::Response::getSupportedeExtensions(const string& key) {
    static map<string, string> ext;
    
    if (ext.empty()) {
        ext[".7z"]    = "Content-Type: application/x-7z-compressed\r\n";
        ext[".avi"]   = "Content-Type: video/x-msvideo\r\n";
        ext[".bat"]   = "Content-Type: application/x-msdownload\r\n";
        ext[".bin"]   = "Content-Type: application/octet-stream\r\n";
        ext[".bmp"]   = "Content-Type: image/bmp\r\n";
        ext[".css"]   = "Content-Type: text/css\r\n";
        ext[".csv"]   = "Content-Type: text/csv\r\n";
        ext[".doc"]   = "Content-Type: application/msword\r\n";
        ext[".docx"]  = "Content-Type: application/vnd.openxmlformats-officedocument.wordprocessingml.document\r\n";
        ext[".dll"]   = "Content-Type: application/octet-stream\r\n";
        ext[".exe"]   = "Content-Type: application/octet-stream\r\n";
        ext[".eot"]   = "Content-Type: application/vnd.ms-fontobject\r\n";
        ext[".gif"]   = "Content-Type: image/gif\r\n";
        ext[".gz"]    = "Content-Type: application/gzip\r\n";
        ext[".html"]  = "Content-Type: text/html\r\n";
        ext[".ico"]   = "Content-Type: image/x-icon\r\n";
        ext[".iso"]   = "Content-Type: application/octet-stream\r\n";
        ext[".jpg"]   = "Content-Type: image/jpeg\r\n";
        ext[".jpeg"]  = "Content-Type: image/jpeg\r\n";
        ext[".json"]  = "Content-Type: application/json\r\n";
        ext[".mjs"]   = "Content-Type: text/javascript\r\n";
        ext[".mp3"]   = "Content-Type: audio/mpeg\r\n";
        ext[".mp4"]   = "Content-Type: video/mp4\r\n";
        ext[".mov"]   = "Content-Type: video/quicktime\r\n";
        ext[".mkv"]   = "Content-Type: video/x-matroska\r\n";
        ext[".ogg"]   = "Content-Type: audio/ogg\r\n";
        ext[".otf"]   = "Content-Type: font/otf\r\n";
        ext[".png"]   = "Content-Type: image/png\r\n";
        ext[".pdf"]   = "Content-Type: application/pdf\r\n";
        ext[".ppt"]   = "Content-Type: application/vnd.ms-powerpoint\r\n";
        ext[".pptx"]  = "Content-Type: application/vnd.openxmlformats-officedocument.presentationml.presentation\r\n";
        ext[".php"]   = "Content-Type: application/x-httpd-php\r\n";
        ext[".py"]    = "Content-Type: text/x-python\r\n";
        ext[".rar"]   = "Content-Type: application/x-rar-compressed\r\n";
        ext[".rtf"]   = "Content-Type: application/rtf\r\n";
        ext[".svg"]   = "Content-Type: image/svg+xml\r\n";
        ext[".sh"]    = "Content-Type: application/x-sh\r\n";
        ext[".sfnt"]  = "Content-Type: font/sfnt\r\n";
        ext[".webm"]  = "Content-Type: video/webm\r\n";
        ext[".woff"]  = "Content-Type: font/woff\r\n";
        ext[".woff2"] = "Content-Type: font/woff2\r\n";
        ext[".xml"]   = "Content-Type: application/xml\r\n";
        ext[".xls"]   = "Content-Type: application/vnd.ms-excel\r\n";
        ext[".xlsx"]  = "Content-Type: application/vnd.openxmlformats-officedocument.spreadsheetml.sheet\r\n";
        ext[".zip"]   = "Content-Type: application/zip\r\n";
        ext[".odt"]   = "Content-Type: application/vnd.oasis.opendocument.text\r\n";
        ext[".ods"]   = "Content-Type: application/vnd.oasis.opendocument.spreadsheet\r\n";
        ext[".odp"]   = "Content-Type: application/vnd.oasis.opendocument.presentation\r\n";
        ext[".txt"]   = "Content-Type: text/plain\r\n";
        ext[".js"]    = "Content-Type: application/javascript\r\n";
        ext[".java"]  = "Content-Type: text/java\r\n";
    }
    if (ext.find(key) != ext.end()) {
        return ext[key];
    }
    return "";
}



string	httpSession::Response::contentTypeHeader() const {
	size_t pos = s.path.rfind(".");
    if (s.showDirFiles == true) {
        return ("Content-Type: text/html\r\n");
    }
	if (pos == string::npos)
        return ("Content-Type: application/octet-stream\r\n");
	string ext = s.path.substr(pos);
	string contentTypeValue = getSupportedeExtensions(ext);
	if (contentTypeValue.empty())
        return ("Content-Type: application/octet-stream\r\n");
	return (contentTypeValue);
}

void	httpSession::Response::sendHeader() {
	string header;

    // cerr << s.statusCode << endl;
	// cerr << s.codeMeaning << endl;
	header += "HTTP/1.1 " + toString(s.statusCode) + " " + s.codeMeaning + "\r\n";
	if (s.method == GET) {
        header += contentTypeHeader();
	    header += "Transfer-Encoding: chunked\r\n";
        s.sstat = ss_sBody;
    }
    if (s.showDirFiles == true)
        s.sstat = ss_sBodyAutoindex;
    if (s.headers["connection"] == "keep-alive")
	    header += "Connection: keep-alive\r\n";
    else
	    header += "Connection: close\r\n";
    if (!s.returnedLocation.empty()) {
        header += "Location: " + s.returnedLocation +"\r\n";
        s.sstat = ss_done;
    }
	header += "Server: bngn/0.1\r\n";
	header += "\r\n";
    cerr << "header" << endl;
    cerr << header << endl;
    cerr << "----" << endl;
	if (send(s.clientFd, header.c_str(), header.size(), MSG_DONTWAIT) <= 0) {
		cerr << "write failed(sendResponse.cpp 24)" << endl;
		s.sstat = ss_cclosedcon;
		return ;
	}
}

void	httpSession::Response::sendBody() {
	char buff[BUFFER_SIZE];
    ssize_t sizeRead;

	if (contentFd == -1) {
		if ((contentFd = open(s.path.c_str(), O_RDONLY, 0644)) == -1) {
			perror("open failed(sendresponse.cpp 37)");
			throw(statusCodeException(500, "Internal Server Error"));
		}
	}
	if((sizeRead = read(contentFd, buff, BUFFER_SIZE)) < 0) {
		perror("read failed(sendresponse.cpp 43)");
		throw(statusCodeException(500, "Internal Server Error"));
	}
	if (sizeRead > 0) {
        bstring body;
		ostringstream chunkSize;
		chunkSize << hex << sizeRead << "\r\n";
        body += chunkSize.str().c_str();
        body += bstring(buff, sizeRead);
        body += "\r\n";
		if (send(s.clientFd, body.c_str(), body.size(), MSG_DONTWAIT) <= 0) {//not good wrapper good
			cerr << "write failed(sendResponse.cpp 50" << endl;
			s.sstat = ss_cclosedcon;
			return ;
		}
	} else {
		if (send(s.clientFd, "0\r\n\r\n", 5, MSG_DONTWAIT) <= 0) {
            cerr << "write failed(sendResponse.cpp 50" << endl;
			s.sstat = ss_cclosedcon;
			return ;
		}
		s.sstat = ss_done;
	}
}







// string httpSession::Response::getExt() {
//     size_t size = s.path.find_last_of(".");
//     string ext;

//     if (size != string::npos) {
//         ext = s.path.substr(size);
//     }
//     return ext;
// }

// void httpSession::Response::Get(int clientFd, bool smallFile) {
//     string response;
//     string fileType = getSupportedeExtensions(getExt());

//     if (smallFile) {
//         ifstream fileStream(s.path.c_str(), std::ios::binary);
//         std::stringstream buffer;
//         buffer << fileStream.rdbuf();
//         string body = buffer.str();

//         response = "HTTP/1.1 200 OK\r\n" + fileType +
//                         "Content-Length: " + toString(body.size()) + "\r\n" + "Connection: " +
//                         getConnection(s.getHeaders()["connection"]) + string("\r\n\r\n");
//         response += body;
//         send(clientFd, response.c_str(), response.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
//         s.sstat = ss_done;
//         if (s.closeAutoIndex == true) {
//             unlink(s.path.c_str());
//         }
//     }
//     else {
//         headerSended = true;
//         contentFd = open(s.path.c_str(), O_RDONLY);
//         response = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n" + fileType + "Connection: " +
//                             getConnection(s.getHeaders()["connection"]) + string("\r\n\r\n");
//         send(clientFd, response.c_str(), response.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
//     }
// }

// void httpSession::Response::sendBodyifChunked(int clientFd) {
//     char buffer[BUFFER_SIZE+1];
//     ssize_t bytesRead = read(contentFd, buffer, BUFFER_SIZE);

//     if (bytesRead < 0) {
//         // cout << contentFd << endl;
//         perror("ba33");
//     }
//     else if (bytesRead > 0) {
//         buffer[bytesRead] = '\0';
//         std::ostringstream BUFFER_SIZEStream;
//         BUFFER_SIZEStream << std::hex << bytesRead << "\r\n";
//         std::string BUFFER_SIZEStr = BUFFER_SIZEStream.str();
//         send(clientFd, BUFFER_SIZEStr.c_str(), BUFFER_SIZEStr.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
//         send(clientFd, buffer, bytesRead, MSG_DONTWAIT | MSG_NOSIGNAL);
//         send(clientFd, "\r\n", 2, MSG_DONTWAIT | MSG_NOSIGNAL);
//     }
//     else {
//         buffer[bytesRead] = '\0';
//         send(clientFd, "0\r\n\r\n", 5, MSG_DONTWAIT | MSG_NOSIGNAL);
//         s.sstat = ss_done;
//         headerSended = false;
//         if (contentFd >= 0) {
//             ft_close(contentFd, "fileFd");
//         }
//         if (s.closeAutoIndex == true) {
//             unlink(s.path.c_str());
//         }
//     }
// }
