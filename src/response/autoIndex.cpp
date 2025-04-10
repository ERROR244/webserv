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
#include <iomanip>  // For std::setw and std::left



std::string generate_autoindex_html(const std::string &dir_path, const std::string &uri_path) {
    DIR *dir = opendir(dir_path.c_str());
    if (!dir) {
        cout << "Failed to open dir\n";
        return "";
    }

    std::ostringstream html_stream;

    html_stream << "<!DOCTYPE html>\n<html>\n<head><title>Index of " << uri_path << "</title></head>\n<body>\n"
                << "<h1>Index of " << uri_path << "</h1><hr><pre>\n";

    // Go up a directory (if it's not the root)
    if (uri_path != "/")
        html_stream << "<a href=\"../\">../</a>\n";

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0) // Skip current directory entry
            continue;

        std::string full_path = dir_path + "/" + entry->d_name;
        struct stat st;
        if (stat(full_path.c_str(), &st) == -1)
            continue;

        // Format the timestamp
        char timebuf[64];
        struct tm *tm = localtime(&st.st_mtime);
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M", tm);

        // Add entry to HTML list
        html_stream << "<a href=\"" << uri_path << "/" << entry->d_name << "\">"
            << std::left << std::setw(32) << entry->d_name << "</a>  "
            << timebuf << "  "
            << (S_ISDIR(st.st_mode) ? "<DIR>" : toString(st.st_size)) << "\n";
    }


    html_stream << "</pre><hr></body>\n</html>\n";
    closedir(dir);
    return html_stream.str();
}

bool write_to_file(const std::string &filename, const std::string &content) {
    std::ofstream file(filename.c_str());
    if (!file) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return false;
    }
    file << content;
    file.close();
    return true;
}