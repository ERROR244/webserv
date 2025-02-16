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

string httpSession::Response::getExt(string path) {
    size_t size = s.path.find_last_of(".");
    string ext;

    if (size != string::npos) {
        ext = s.path.substr(size);
    }
    return ext;
}

void httpSession::Response::GET(int clientFd, bool smallFile) {
    string response;
    string fileType = getSupportedeExtensions(getExt(s.path));

    // cout << s.path << endl;
    if (smallFile) {
        ifstream fileStream(s.path.c_str(), std::ios::binary);
        std::stringstream buffer;
        buffer << fileStream.rdbuf();
        string body = buffer.str();
        response = "HTTP/1.1 200 OK\r\n" + fileType +
                        "Content-Length: " + toString(body.size()) + "\r\n" +
                        "Connection: keep-alive" + string("\r\n\r\n");
        response += body;
        send(clientFd, response.c_str(), response.size(), MSG_DONTWAIT);
        state = DONE;
        cout << "\ndone sending the response\n" << endl;
        // lastRes = time(nullptr);      // for timeout
    }
    else {
        headerSended = true;
        contentFd = open(s.path.c_str(), O_RDONLY);
        response =   "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n" +
                            fileType + "Connection: keep-alive" + string("\r\n\r\n");
        send(clientFd, response.c_str(), response.size(), MSG_DONTWAIT);
    }
}

void httpSession::Response::sendBodyifChunked(int clientFd) {
    char buffer[BUFFER_SIZE+1];
    ssize_t bytesRead = read(contentFd, buffer, BUFFER_SIZE);

    if (bytesRead < 0) {
        cout << contentFd << endl;
        perror("ba33");
        exit(1);
    }
    else if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        std::ostringstream BUFFER_SIZEStream;
        BUFFER_SIZEStream << std::hex << bytesRead << "\r\n";
        std::string BUFFER_SIZEStr = BUFFER_SIZEStream.str();
        send(clientFd, BUFFER_SIZEStr.c_str(), BUFFER_SIZEStr.size(), MSG_DONTWAIT);
        send(clientFd, buffer, bytesRead, MSG_DONTWAIT);
        send(clientFd, "\r\n", 2, MSG_DONTWAIT);
    }
    else {
        buffer[bytesRead] = '\0';
        send(clientFd, "0\r\n\r\n", 5, MSG_DONTWAIT);
                state = DONE;

        if (contentFd >= 0)
            ft_close(contentFd, "fileFd");
        headerSended = false;
        // lastRes = time(nullptr);      // for timeout
        cout << "\ndone sending the response\n" << endl;
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
