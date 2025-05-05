#include "httpSession.hpp"

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


