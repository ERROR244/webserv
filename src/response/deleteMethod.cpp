#include "httpSession.hpp"

string httpSession::Response::deleteFile(const string& file, const string& connection) {
    string response;

    unlink(file.c_str());
    response = "HTTP/1.1 204 No Content\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: 0\r\n";
    response += "Connection: " + connection;
    response += "\r\n\r\n";
    return response;
}

string httpSession::Response::deleteDir(const string& dir, const string& connection) {
    string response = "";

    if (remove(dir.c_str()) != 0) {
        std::cerr << "Deleting Non-Empty Directory.\n";
        response += "HTTP/1.1 409 Conflict\r\n";
        response += "Content-Type: text/html\r\n";
        response += "content-length: 134\r\n";
        response += "Connection: " + connection;
        response += "\r\n\r\n<!DOCTYPE html><html><head><title>409 Conflict</title></head><body><h1>Conflict</h1><p>Deleting Non-Empty Directory.</p></body></html>";
    }
    else {
        response += "HTTP/1.1 204 No Content\r\n";
        response += "Content-Type: text/html\r\n";
        response += "Content-Length: 0\r\n";
        response += "Connection: " + connection;
        response += "\r\n\r\n";
    }
    return response;
}

string httpSession::Response::getDeleteRes(const string& path, const string& connection, struct stat& file_stat) {
    string response = "";
    string body;

    if (path.find(s.rules->uploads) == string::npos) {
        std::cerr << "Directory Traversal Attempt While Deleting.\n";
        body = "<!DOCTYPE html><html><head><title>409 Conflict</title></head><body><h1>Conflict</h1><p>you don't have access to resource.</p></body></html>";
        response += "HTTP/1.1 403 Forbidden\r\n";
    }
    else if (!(file_stat.st_mode & S_IWUSR)) {
        body = "<!DOCTYPE html><html><head><title>403 Forbidden</title></head><body><h1>Forbidden</h1><p>user doesn't have write permission.</p></body></html>";
        response += "HTTP/1.1 403 Forbidden\r\n";
    }
    else if (S_ISDIR(file_stat.st_mode) != 0) {
        return (deleteDir(path, connection));
    }
    else if (S_ISREG(file_stat.st_mode) != 0) {
        return (deleteFile(path, connection));
    }
    else {
        body = "<!DOCTYPE html><html><head><title>403 Forbidden</title></head><body><h1>Forbidden</h1><p>resource is not a file or directory.</p></body></html>";
        response += "HTTP/1.1 403 Forbidden\r\n";
    }
    response += "Content-Type: text/html\r\n";
    response += "content-length: " + toString(body.size()) + "\r\n";
    response += "Connection: " + connection + "\r\n\r\n";
    response += body;
    return response;
}
