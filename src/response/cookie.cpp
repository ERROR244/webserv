#include <sstream>
#include "httpSession.hpp"

string getSessionID(const string& cookieId) {
    if (!cookieId.empty()) {
        size_t pos = cookieId.find("sessionID=");
        if (pos != string::npos) {
            pos += 10;
            size_t end = cookieId.find(";", pos);
            return (end == string::npos) ? cookieId.substr(pos) : cookieId.substr(pos, end - pos);
        }
    }
    return "";
}

void setCookie(string& sessionId, const string& cookieId) {
    sessionId = getSessionID(cookieId);
    if (sessionId.empty()) {
        sessionId = generateSessionID();
    }
    // cout << "cookie from EPOLLIN:	" << cookieId << endl;
    // cout << "sessionId from EPOLLIN:	" << sessionId << endl;
}

string generateSessionID() {
    stringstream ss;
    srand(time(0));

    for (int i = 0; i < 32; ++i) {
        int randomValue = rand() % 16;
        ss << hex << randomValue;
    }
    return ss.str();
}

string getSessionCookie(string& sessionID) {
    if (sessionID.empty())
        sessionID = generateSessionID();

    return "\r\nSet-Cookie: sessionID=" + sessionID + "; Path=/; HttpOnly";
}
