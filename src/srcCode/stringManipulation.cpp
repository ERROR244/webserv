#include "webServ.hpp"

string toString(const int& nbr) {
    ostringstream oss;

    oss << nbr;
    return (oss.str());
}

u_map webServ::getSupportedeExtensions() {
    u_map ext;

    ext[".html"] = "Content-Type: text/html\r\n";
    ext[".htm"]  = "Content-Type: text/html\r\n";
    ext[".css"]  = "Content-Type: text/css\r\n";
    ext[".js"]   = "Content-Type: text/javascript\r\n";
    ext[".mjs"]  = "Content-Type: text/javascript\r\n";
    ext[".csv"]  = "Content-Type: text/csv\r\n";
    ext[".png"]  = "Content-Type: images/png\r\n";
    ext[".txt"]  = "Content-Type: text/plan\r\n";
    ext[".jpg"]  = "Content-Type: images/jpeg\r\n";
    ext[".avi"]   = "Content-Type: video/x-msvideo\r\n";
    ext[".ico"]   = "Content-Type: image/x-icon\r\n";
    ext[".gif"]   = "Content-Type: image/gif\r\n";
    ext[".jpeg"]  = "Content-Type: image/jpeg\r\n";
    ext[".svg"]   = "Content-Type: image/svg+xml\r\n";
    ext[".webp"]  = "Content-Type: image/webp\r\n";
    ext[".bmp"]   = "Content-Type: image/bmp\r\n";
    ext[".tiff"]  = "Content-Type: image/tiff\r\n";
    ext[".json"]  = "Content-Type: application/json\r\n";
    ext[".xml"]   = "Content-Type: application/xml\r\n";
    ext[".pdf"]   = "Content-Type: application/pdf\r\n";
    ext[".zip"]   = "Content-Type: application/zip\r\n";
    ext[".tar"]   = "Content-Type: application/x-tar\r\n";
    ext[".gz"]    = "Content-Type: application/gzip\r\n";
    ext[".rar"]   = "Content-Type: application/x-rar-compressed\r\n";
    ext[".7z"]    = "Content-Type: application/x-7z-compressed\r\n";
    ext[".mp3"]   = "Content-Type: audio/mpeg\r\n";
    ext[".wav"]   = "Content-Type: audio/wav\r\n";
    ext[".ogg"]   = "Content-Type: audio/ogg\r\n";
    ext[".mp4"]   = "Content-Type: video/mp4\r\n";
    ext[".webm"]  = "Content-Type: video/webm\r\n";
    ext[".mov"]   = "Content-Type: video/quicktime\r\n";
    ext[".mkv"]   = "Content-Type: video/x-matroska\r\n";
    ext[".doc"]   = "Content-Type: application/msword\r\n";
    ext[".docx"]  = "Content-Type: application/vnd.openxmlformats-officedocument.wordprocessingml.document\r\n";
    ext[".xls"]   = "Content-Type: application/vnd.ms-excel\r\n";
    ext[".xlsx"]  = "Content-Type: application/vnd.openxmlformats-officedocument.spreadsheetml.sheet\r\n";
    ext[".ppt"]   = "Content-Type: application/vnd.ms-powerpoint\r\n";
    ext[".pptx"]  = "Content-Type: application/vnd.openxmlformats-officedocument.presentationml.presentation\r\n";
    ext[".odt"]   = "Content-Type: application/vnd.oasis.opendocument.text\r\n";
    ext[".ods"]   = "Content-Type: application/vnd.oasis.opendocument.spreadsheet\r\n";
    ext[".odp"]   = "Content-Type: application/vnd.oasis.opendocument.presentation\r\n";
    ext[".rtf"]   = "Content-Type: application/rtf\r\n";
    ext[".php"]   = "Content-Type: application/x-httpd-php\r\n";
    ext[".py"]    = "Content-Type: text/x-python\r\n";
    ext[".java"]  = "Content-Type: text/x-java-source\r\n";
    ext[".sh"]    = "Content-Type: application/x-sh\r\n";
    ext[".bat"]   = "Content-Type: application/x-msdownload\r\n";
    ext[".exe"]   = "Content-Type: application/octet-stream\r\n";
    ext[".dll"]   = "Content-Type: application/octet-stream\r\n";
    ext[".iso"]   = "Content-Type: application/octet-stream\r\n";
    ext[".bin"]   = "Content-Type: application/octet-stream\r\n";
    ext[".woff"]  = "Content-Type: font/woff\r\n";
    ext[".woff2"] = "Content-Type: font/woff2\r\n";
    ext[".ttf"]   = "Content-Type: font/ttf\r\n";
    ext[".otf"]   = "Content-Type: font/otf\r\n";
    ext[".eot"]   = "Content-Type: application/vnd.ms-fontobject\r\n";
    ext[".sfnt"]  = "Content-Type: font/sfnt\r\n";
    return ext;
}

string webServ::getFile(string str) {
    istringstream  iss(str);
    string              path;
    string              ext;

    iss >> path;
    iss >> path;
    if (path == "/") {
        return ("./data/index.html");
    }
    size_t size = path.find_last_of(".");
    fileType = "";
    if (size != string::npos) {
        ext = path.substr(size);
        if (extensions.find(ext) != extensions.end()) {
            fileType =  extensions[ext];
        }
    }
    return ("." + path);
}

string webServ::getBody(string str) {
    string  body;
    size_t  index = 0;

    while (index < str.size()) {
        if (index + 3 < str.size())
            break ;
        else if (str[index] == '\r' && str[index + 1] == '\n' && 
            str[index + 2] == '\r' && str[index + 3] == '\n') {
            index += 3;
            break;
        }
        index++;
    }
    body = str.substr(index);
    return (body);
}

vector<string> webServ::split_string(const string& str, const string& delimiters) {  
    vector<string> tokens;  
    string token;  
    size_t start = 0;  
    size_t end;  

    while ((end = str.find_first_of(delimiters, start)) != string::npos) {  
        if (end > start) {  
            tokens.push_back(str.substr(start, end - start));
        }  
        tokens.push_back(string(1, str[end]));
        start = end + 1;  
    }  
    
    if (start < str.length()) {  
        tokens.push_back(str.substr(start));
    }  

    return tokens;  
}

