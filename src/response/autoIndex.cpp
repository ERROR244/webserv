#include "httpSession.hpp"
#include <dirent.h>

static void replace(string& str, const string& from, const string& to) {
    size_t start_pos;
    while ((start_pos = str.find(from)) != string::npos) {    
        str.replace(start_pos, from.length(), to);
    }
}

static string addHrefs(string path) {
    string  hrefs;
    string  hrefsTemplate = "<a href=\"{{NAME}}\">{{NAME}}/</a>\n";
    DIR* dir = opendir(path.c_str());
    if (dir == NULL) {
        cerr << "Could not open directory: " << path << endl;
        return "";
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (string(entry->d_name) == ".")
            continue;
        string tmp(hrefsTemplate);
        replace(tmp, "{{NAME}}", entry->d_name);
        if (string(entry->d_name) == "..")
            hrefs = tmp + hrefs;
        else
            hrefs += tmp;
    }
    closedir(dir);
    return hrefs;
}

void httpSession::Response::generateHtml() {
    string html =
        "<html>\n"
        "<head>\n"
        "    <title>{{TITLE}}</title>\n"
        "</head>\n"
        "<body>\n"
        "    <h1>Index of {{TITLE}}</h1>\n"
        "    <hr>\n"
        "    <pre>\n";
    replace(html, "{{TITLE}}", s.rawUri);
    string hrefs = addHrefs(s.path);
    if (hrefs.empty()) {
        s.sstat = ss_cclosedcon;
        return;
    }
    html += hrefs;
    html +=
        "   </pre>\n"
        "   <hr>\n"
        "</body>\n"
        "</html>\n";
    ostringstream chunkSize;
	chunkSize << hex << html.size() << "\r\n";
    bstring body(chunkSize.str().c_str(), chunkSize.str().size());
    body += bstring(html.c_str(), html.size());
    body += "\r\n";
    body += "0\r\n\r\n";
    if (send(s.clientFd, body.c_str(), body.size(), MSG_DONTWAIT | MSG_NOSIGNAL) <= 0) {
        cerr << "send failed" << endl;
        s.sstat = ss_cclosedcon;
        return ;
    }
    s.sstat = ss_done;
}