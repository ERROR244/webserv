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
        ext[".js"]    = "Content-Type: text/javascript\r\n";
        ext[".jpg"]   = "Content-Type: images/jpeg\r\n";
        ext[".jpeg"]  = "Content-Type: image/jpeg\r\n";
        ext[".json"]  = "Content-Type: application/json\r\n";
        ext[".java"]  = "Content-Type: text/x-java-source\r\n";
        ext[".mjs"]   = "Content-Type: text/javascript\r\n";
        ext[".mp3"]   = "Content-Type: audio/mpeg\r\n";
        ext[".mp4"]   = "Content-Type: video/mp4\r\n";
        ext[".mov"]   = "Content-Type: video/quicktime\r\n";
        ext[".mkv"]   = "Content-Type: video/x-matroska\r\n";
        ext[".ogg"]   = "Content-Type: audio/ogg\r\n";
        ext[".odt"]   = "Content-Type: application/vnd.oasis->opendocument.text\r\n";
        ext[".ods"]   = "Content-Type: application/vnd.oasis->opendocument.spreadsheet\r\n";
        ext[".odp"]   = "Content-Type: application/vnd.oasis->opendocument.presentation\r\n";
        ext[".otf"]   = "Content-Type: font/otf\r\n";
        ext[".png"]   = "Content-Type: images/png\r\n";
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
        ext[".txt"]   = "Content-Type:getSessionCookie audio/wav\r\n";
        ext[".webm"]  = "Content-Type: video/webm\r\n";
        ext[".woff"]  = "Content-Type: font/woff\r\n";
        ext[".woff2"] = "Content-Type: font/woff2\r\n";
        ext[".xml"]   = "Content-Type: application/xml\r\n";
        ext[".xls"]   = "Content-Type: application/vnd.ms-excel\r\n";
        ext[".xlsx"]  = "Content-Type: application/vnd.openxmlformats-officedocument.spreadsheetml.sheet\r\n";
        ext[".zip"]   = "Content-Type: application/zip\r\n";
    }
    if (ext.find(key) != ext.end()) {
        return ext[key];
    }
    return "";
}

string httpSession::Response::getExt(string path) {
    (void)path;
    size_t size = s.path.find_last_of(".");
    string ext;

    if (size != string::npos) {
        ext = s.path.substr(size);
    }
    return ext;
}

void httpSession::Response::Get(int clientFd, bool smallFile) {
    string response;
    string fileType = getSupportedeExtensions(getExt(s.path));

    if (smallFile) {
        ifstream fileStream(s.path.c_str(), std::ios::binary);
        std::stringstream buffer;
        buffer << fileStream.rdbuf();
        string body = buffer.str();
        response = "HTTP/1.1 200 OK\r\n" + fileType +
                        "Content-Length: " + toString(body.size()) + "\r\n" + "Connection: " +
                        (s.headers["connection"].empty() ? "close" : s.headers["connection"]) +
                        getSessionCookie(s.sessionId) +
                        string("\r\n\r\n");
        response += body;
        send(clientFd, response.c_str(), response.size(), MSG_DONTWAIT);
        s.sstat = done;
        lastActivityTime = time(NULL);      // for timeout
    }
    else {
        headerSended = true;
        contentFd = open(s.path.c_str(), O_RDONLY);
        response = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n" + fileType + "Connection: " +
                            (s.headers["connection"].empty() ? "close" : s.headers["connection"]) +
                            getSessionCookie(s.sessionId) +
                            string("\r\n\r\n");
        send(clientFd, response.c_str(), response.size(), MSG_DONTWAIT);
    }
}

void httpSession::Response::sendBodyifChunked(int clientFd) {
    char buffer[BUFFER_SIZE+1];
    ssize_t bytesRead = read(contentFd, buffer, BUFFER_SIZE);

    if (bytesRead < 0) {
        cout << contentFd << endl;
        perror("ba33");
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
        s.sstat = done;
        headerSended = false;
        if (contentFd >= 0)
            ft_close(contentFd, "fileFd");
        lastActivityTime = time(NULL);      // for timeout
    }
}
