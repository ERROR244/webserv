#include "httpSession.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>
#include <ctime>
#include <cstring>
#include <iomanip>
#include <iomanip>  // For setw and left

void replace(string& str, const string& from, const string& to) {
    size_t start_pos;
    while ((start_pos = str.find(from)) != string::npos) {    
        str.replace(start_pos, from.length(), to);
    }
}

string addHrefs(string path) {
    string  hrefs;
    string  hrefsTemplate = "<a href=\"{{NAME}}\">{{NAME}}/</a>\n";
    // "<a href=\"{{NAME}}\">{{NAME}}/</a> {{TIME}} {{bytes}}\n"
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
    cerr << body << endl;
    if (send(s.clientFd, body.c_str(), body.size(), MSG_DONTWAIT) <= 0) {
        cerr << "send failed" << endl;
        s.sstat = ss_cclosedcon;
        return ;
    }
    s.sstat = ss_done;
}






















// string get_name(string name, bool is_not_a_file) {
//     if (name.length() + 1 > 50) {
//         return name.substr(0, 47) + "..&gt;" + "</a>";
//     } else if (is_not_a_file) {
//         return name + "/" + "</a>";
//     }
//     return name + "</a>";
// }


// string generate_autoindex_html(const string &dir_path, const string &uri_path) {
//     ostringstream html_stream;
//     struct dirent *entry;
//     vector<string> dir_links;
//     vector<string> files_links;
//     ostringstream link;
//     char timebuf[64];
//     struct tm *tm;
//     struct stat st;
//     string name;
//     DIR *dir = opendir(dir_path.c_str());

//     if (!dir) {
//         cout << "Failed to open dir\n";
//         return "";
//     }


//     html_stream << "<html>\n"
//                 << "<head><title>Index of " << uri_path << "</title></head>\n"
//                 << "<body>\n"
//                 << "<h1>Index of " << uri_path << "</h1><hr><pre>\n";

//     while ((entry = readdir(dir)) != NULL) {
//         if (strcmp(entry->d_name, ".") == 0 || stat((dir_path + "/" + entry->d_name).c_str(), &st) == -1)
//             continue;

//         tm = localtime(&st.st_mtime);
//         strftime(timebuf, sizeof(timebuf), "%d-%b-%Y %H:%M", tm);

//         name = get_name(string(entry->d_name), S_ISDIR(st.st_mode));
//         link << "<a href=\"" << uri_path << entry->d_name << "/\">" << name << setw(72 - name.size());
//         if (name != "../</a>") {
//             link << timebuf << "                   " << (S_ISDIR(st.st_mode) ? "-" : toString(st.st_size)) << "\n";
//         }
//         else
//             link << "\n";

//         if (S_ISDIR(st.st_mode))
//             dir_links.push_back(link.str());
//         else
//             files_links.push_back(link.str());
//         link.str("");
//         link.clear();
//     }

//     sort(dir_links.begin(), dir_links.end());
//     sort(files_links.begin(), files_links.end());

//     for (size_t i = 0; i < dir_links.size(); ++i) {
//         html_stream << dir_links[i];
//     }
//     for (size_t i = 0; i < files_links.size(); ++i) {
//         html_stream << files_links[i];
//     }

//     html_stream << "</pre><hr></body>\n</html>\n";
//     closedir(dir);
//     return html_stream.str();
// }

// bool write_to_file(const string &filename, const string &content) {
//     ofstream file(filename.c_str());
//     if (!file) {
//         cerr << "Failed to open file for writing: " << filename << endl;
//         return false;
//     }
//     file << content;
//     file.close();
//     return true;
// }