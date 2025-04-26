#include "httpSession.hpp"

// string httpSession::Response::deleteFile(const string& file, const string& connection) {
//     string response;

//     unlink(file.c_str());
//     response = "HTTP/1.1 204 No Content\r\n";
//     response += "Content-Type: text/html\r\n";
//     response += "Content-Length: 0\r\n";
//     response += "Connection: " + connection;
//     response += "\r\n\r\n";
//     return response;
// }

// string httpSession::Response::deleteDir(const string& dir, const string& connection) {
//     string response = "";

//     if (remove(dir.c_str()) != 0) {
//         std::cerr << "Deleting Non-Empty Directory.\n";
//         response += "HTTP/1.1 409 Conflict\r\n";
//         response += "Content-Type: text/html\r\n";
//         response += "content-length: 134\r\n";
//         response += "Connection: " + connection;
//         response += "\r\n\r\n<!DOCTYPE html><html><head><title>409 Conflict</title></head><body><h1>Conflict</h1><p>Deleting Non-Empty Directory.</p></body></html>";
//     }
//     else {
//         response += "HTTP/1.1 204 No Content\r\n";
//         response += "Content-Type: text/html\r\n";
//         response += "Content-Length: 0\r\n";
//         response += "Connection: " + connection;
//         response += "\r\n\r\n";
//     }
//     return response;
// }

void httpSession::Response::deleteContent() {
    struct stat fileStat;
    if (stat(s.path.c_str(), &fileStat)) {
        throw(statusCodeException(404, "Not Found"));
    }
    if (S_ISDIR(fileStat.st_mode) != 0) {
        if (remove(s.path.c_str()) != 0)
            throw(statusCodeException(409, "Conflict"));
    }
    else if (S_ISREG(fileStat.st_mode) != 0) {
        unlink(s.path.c_str());
    }
    else {
        throw(statusCodeException(403, "Forbidden2"));
    }
}


