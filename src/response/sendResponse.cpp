#include "httpSession.hpp"

string httpSession::Response::getSupportedeExtensions(const string& key) {
    static map<string, string> ext;
    
    if (ext.empty()) {
        ext[".7z"]    = "application/x-7z-compressed";
        ext[".avi"]   = "video/x-msvideo";
        ext[".bat"]   = "application/x-msdownload";
        ext[".bin"]   = "application/octet-stream";
        ext[".bmp"]   = "image/bmp";
        ext[".css"]   = "text/css";
        ext[".csv"]   = "text/csv";
        ext[".doc"]   = "application/msword";
        ext[".docx"]  = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
        ext[".dll"]   = "application/octet-stream";
        ext[".exe"]   = "application/octet-stream";
        ext[".eot"]   = "application/vnd.ms-fontobject";
        ext[".gif"]   = "image/gif";
        ext[".gz"]    = "application/gzip";
        ext[".html"]  = "text/html";
        ext[".ico"]   = "image/x-icon";
        ext[".iso"]   = "application/octet-stream";
        ext[".js"]    = "text/javascript";
        ext[".jpg"]   = "images/jpeg";
        ext[".jpeg"]  = "image/jpeg";
        ext[".json"]  = "application/json";
        ext[".java"]  = "text/x-java-source";
        ext[".mjs"]   = "text/javascript";
        ext[".mp3"]   = "audio/mpeg";
        ext[".mp4"]   = "video/mp4";
        ext[".mov"]   = "video/quicktime";
        ext[".mkv"]   = "video/x-matroska";
        ext[".ogg"]   = "audio/ogg";
        ext[".odt"]   = "application/vnd.oasis->opendocument.text";
        ext[".ods"]   = "application/vnd.oasis->opendocument.spreadsheet";
        ext[".odp"]   = "application/vnd.oasis->opendocument.presentation";
        ext[".otf"]   = "font/otf";
        ext[".png"]   = "images/png";
        ext[".pdf"]   = "application/pdf";
        ext[".ppt"]   = "application/vnd.ms-powerpoint";
        ext[".pptx"]  = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
        ext[".php"]   = "application/x-httpd-php";
        ext[".py"]    = "text/x-python";
        ext[".rar"]   = "application/x-rar-compressed";
        ext[".rtf"]   = "application/rtf";
        ext[".svg"]   = "image/svg+xml";
        ext[".sh"]    = "application/x-sh";
        ext[".sfnt"]  = "font/sfnt";
        ext[".txt"]   = "text/plain";
        ext[".tiff"]  = "image/tiff";
        ext[".tar"]   = "application/x-tar";
        ext[".ttf"]   = "font/ttf";
        ext[".webp"]  = "image/webp";
        ext[".wav"]   = "audio/wav";
        ext[".webm"]  = "video/webm";
        ext[".woff"]  = "font/woff";
        ext[".woff2"] = "font/woff2";
        ext[".xml"]   = "application/xml";
        ext[".xls"]   = "application/vnd.ms-excel";
        ext[".xlsx"]  = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
        ext[".zip"]   = "application/zip";
    }
    if (ext.find(key) != ext.end())
        return ext[key];
    return "";
}

string	httpSession::Response::contentTypeHeader() const {
	size_t pos = s.path.rfind(".");
	if (pos == string::npos)
		throw(statusCodeException(501, "Not Implemented"));
	string ext = s.path.substr(pos);
	string contentTypeValue = getSupportedeExtensions(ext);
	if (contentTypeValue.empty())
		throw(statusCodeException(501, "Not Implemented"));//check this before begining to write in the sock
	return ("Content-Type: " + contentTypeValue + "\r\n");
}

void	httpSession::Response::sendHeader(const int clientFd) {
	string header;

	header += s.httpProtocole + " " + to_string(s.statusCode) + " " + s.codeMeaning + "\r\n";
    if (s.method != "POST") {
	    header += contentTypeHeader();
	    header += "Transfer-Encoding: chunked\r\n";
    } else
        header += "Content-Length: 0\r\n";
	header += "Connection: keep-alive\r\n";
	header += "Server: bngn/0.1\r\n";
	header += "\r\n";
	if (write(clientFd, header.c_str(), header.size()) <= 0) {
		perror("write failed(sendResponse.cpp 24)");
		state = CCLOSEDCON;
		return ;
	}
}

void	httpSession::Response::sendBody(const int clientFd) {
	char buff[BUFFER_SIZE+1] = {0};

	if (contentFd == -1) {
		if ((contentFd = open(s.path.c_str(), O_RDONLY, 0644)) == -1) {
			perror("open failed(sendresponse.cpp 37)");
			throw(statusCodeException(500, "Internal Server Error"));
		}
	}
	int sizeRead;
	if((sizeRead = read(contentFd, buff, BUFFER_SIZE)) < 0) {
		perror("read failed(sendresponse.cpp 43)");
		throw(statusCodeException(500, "Internal Server Error"));
	}
	if (sizeRead > 0) {
		ostringstream chunkSize;
		chunkSize << hex << sizeRead << "\r\n";
		if (write(clientFd, chunkSize.str().c_str(), chunkSize.str().size()) <= 0) {//not good wrapper good
			perror("write failed(sendResponse.cpp 50)");
			state = CCLOSEDCON;
			return ;
		}
		if (write(clientFd, buff, sizeRead) <= 0) {
			perror("write failed(sendResponse.cpp 50)");
			state = CCLOSEDCON;
			return ;
		}
		if (write(clientFd, "\r\n", 2) <= 0) {
			perror("write failed(sendResponse.cpp 50)");
			state = CCLOSEDCON;
			return ;
		}
	} else {
		if (write(clientFd, "0\r\n\r\n", 5) <= 0) {
			perror("write failed(sendResponse.cpp 56)");
			state = CCLOSEDCON;
			return ;
		}
		state = DONE;
		close(contentFd);
		cerr << "done sending response" << endl;
	}
}

void    httpSession::Response::sendCgiStarterLine(const int clientFd) {
    string starterLine = s.httpProtocole + " " + to_string(s.statusCode) + " " + s.codeMeaning + "\r\n";
    if (write(clientFd, starterLine.c_str(), starterLine.size()) <= 0) {
		perror("write failed(sendResponse.cpp 143)");
		state = CCLOSEDCON;
	}
}

void    httpSession::Response::sendCgiOutput(const int clientFd) {
    char    buff[BUFFER_SIZE+1] = {0};
    int     byteRead;
    if ((byteRead = read(s.cgi->rFd(), buff, BUFFER_SIZE)) < 0) {
        perror("read failed(sendResponse.cpp 152)");
        throw(statusCodeException(500, "Internal Server Error"));
    }
    cerr << buff << endl;
    if (byteRead > 0) {
        if (write(clientFd, buff, byteRead) <= 0) {
			perror("write failed(sendResponse.cpp 157)");
			state = CCLOSEDCON;
			return ;
		}
    } else {
        state = DONE;
		close(s.cgi->rFd());
		cerr << "done sending response" << endl;
    }
}