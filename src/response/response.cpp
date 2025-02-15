#include "httpSession.hpp"

e_map getSupportedeExtensions() {
    e_map ext;

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
    ext[".htm"]   = "Content-Type: text/html\r\n";
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
    ext[".odt"]   = "Content-Type: application/vnd.oasis.opendocument.text\r\n";
    ext[".ods"]   = "Content-Type: application/vnd.oasis.opendocument.spreadsheet\r\n";
    ext[".odp"]   = "Content-Type: application/vnd.oasis.opendocument.presentation\r\n";
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
    ext[".txt"]   = "Content-Type: text/plan\r\n";
    ext[".tiff"]  = "Content-Type: image/tiff\r\n";
    ext[".tar"]   = "Content-Type: application/x-tar\r\n";
    ext[".ttf"]   = "Content-Type: font/ttf\r\n";
    ext[".webp"]  = "Content-Type: image/webp\r\n";
    ext[".wav"]   = "Content-Type: audio/wav\r\n";
    ext[".webm"]  = "Content-Type: video/webm\r\n";
    ext[".woff"]  = "Content-Type: font/woff\r\n";
    ext[".woff2"] = "Content-Type: font/woff2\r\n";
    ext[".xml"]   = "Content-Type: application/xml\r\n";
    ext[".xls"]   = "Content-Type: application/vnd.ms-excel\r\n";
    ext[".xlsx"]  = "Content-Type: application/vnd.openxmlformats-officedocument.spreadsheetml.sheet\r\n";
    ext[".zip"]   = "Content-Type: application/zip\r\n";
    return ext;
}

httpSession::Response::Response(httpSession& session) : s(session), contentFd(-1), state(PROCESSING) {
	extensions = getSupportedeExtensions();
}

void	httpSession::Response::sendResponse(const int clientFd) {
	if (s.cgi == NULL) {
		if (state == PROCESSING) {
			sendHeader(clientFd);
			if (state == CCLOSEDCON)
				return ;
			state = SHEADER;
		}
		if (s.method != "POST")
			sendBody(clientFd);
		else
			state = DONE;
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

