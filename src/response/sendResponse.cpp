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
    return "Content-Type: application/octet-stream\r\n";
}

static bool ft_send(int __fd, const void *__buf, size_t __n, e_sstat& status) {
    if (send(__fd, __buf, __n, MSG_DONTWAIT | MSG_NOSIGNAL) <= 0) {
        cerr << "send failed" << endl;
        status = ss_cclosedcon;
        return false;
    }
    return true;
}

string	httpSession::Response::contentTypeHeader() const {
    if (s.showDirFiles == true) {
        return ("Content-Type: text/html\r\n");
    }
	size_t pos = s.path.rfind(".");
	if (pos == string::npos)
        return ("Content-Type: application/octet-stream\r\n");
	string ext = s.path.substr(pos);
	string contentTypeValue = getSupportedeExtensions(ext);
	return (contentTypeValue);
}

void	httpSession::Response::sendHeader() {
	string header;

	header += "HTTP/1.1 " + toString(s.statusCode) + " " + s.codeMeaning + "\r\n";
	switch (s.method) {
    case GET: 
    {
        if (s.returnedLocation.empty()) {
            header += contentTypeHeader();
	        header += "Transfer-Encoding: chunked\r\n";
        }
        s.sstat = ss_sBody;
        break;
    }
    case DELETE:
    {
        deleteContent();
        s.sstat = ss_done;
        break;
    }
    case POST: {
        s.sstat = ss_done;
        break;
    }
    default:
        break;
    }
    if (s.showDirFiles == true)
        s.sstat = ss_sBodyAutoindex;
    if (getHeaderValue(s.headers, "connection") == "keep-alive")
	    header += "Connection: keep-alive\r\n";
    else
	    header += "Connection: close\r\n";
    if (!s.returnedLocation.empty()) {
        header += "Location: " + s.returnedLocation +"\r\n";
        s.sstat = ss_done;
    }
	header += "\r\n";
	ft_send(s.clientFd, header.c_str(), header.size(), s.sstat);
}

void	httpSession::Response::sendBody() {
	char        buff[BUFFER_SIZE] = {0};
    streamsize sizeRead;
	
    if (inputFile.is_open() == false) {
        inputFile.open(s.path.c_str(), ios::binary);
        if (inputFile.is_open() == false) {
            cerr << "open failed" << endl;
            s.sstat = ss_cclosedcon;
            return;
        }
	}
    inputFile.read(buff, BUFFER_SIZE);
    sizeRead =  inputFile.gcount();
    if (sizeRead > 0) {
		ostringstream chunkSize;
		chunkSize << hex << sizeRead << "\r\n";
		if ( ft_send(s.clientFd, chunkSize.str().c_str(), chunkSize.str().size(), s.sstat) == false ||
             ft_send(s.clientFd, buff, sizeRead, s.sstat) == false ||
             ft_send(s.clientFd, "\r\n", 2, s.sstat) == false ) {
			return ;
		}
	} else if (sizeRead == 0) {
		if (ft_send(s.clientFd, "0\r\n\r\n", 5, s.sstat) == false) {
			return ;
		}
        inputFile.close();
		s.sstat = ss_done;
	}
    else {
        s.sstat = ss_cclosedcon;
    }
}
