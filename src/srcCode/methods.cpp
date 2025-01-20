#include "webServ.hpp"

// Transfer-Encoding: chunked
string webServ::GET(const string& requestedFile) {
    ifstream fileStream(requestedFile.c_str(), std::ios::binary);
    struct stat file_stat;
    if (stat(requestedFile.c_str(), &file_stat) != 0) {
        std::cerr << "Could not open the file: " << requestedFile + "\n";
        statusCode = 404;
        return "";
    }
    if (!(file_stat.st_mode & S_IRUSR)) {
        std::cerr << "user don't have Permissions.\n";
        statusCode = 403;
        return "";
    }

    if (file_stat.st_size < 10000) {
        std::stringstream buffer;
        buffer << fileStream.rdbuf();
        string body = buffer.str();
        string response = "HTTP/1.1 200 OK\r\n" + fileType +
                        "Content-Length: " + toString(body.size()) + "\r\n" +
                        "Connection: close" + string("\r\n\r\n");
        cout << "HERE--->" << response << endl;
        response += body;
        send(clientFd, response.c_str(), response.size(), MSG_DONTWAIT);
        return response;
    }
    else {
        string response =   "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n" +
                            fileType + "Connection: close" + string("\r\n\r\n");
        cout << "HERE--->" << response << endl;
        send(clientFd, response.c_str(), response.size(), MSG_DONTWAIT);
        
        const size_t chunkSize = 10000;
        char buffer[chunkSize];
        int i = 0;
        while(!fileStream.eof()) {
            fileStream.read(buffer, chunkSize);
            streamsize bytesRead = fileStream.gcount();
            
            i += bytesRead;
            if (bytesRead > 0) {
                std::ostringstream chunkSizeStream;
                chunkSizeStream << std::hex << bytesRead << "\r\n";
                std::string chunkSizeStr = chunkSizeStream.str();

                send(clientFd, chunkSizeStr.c_str(), chunkSizeStr.size(), MSG_DONTWAIT);

                send(clientFd, buffer, bytesRead, MSG_DONTWAIT);

                send(clientFd, "\r\n", 2, MSG_DONTWAIT);
            }
        }
        send(clientFd, "0\r\n\r\n", 5, MSG_DONTWAIT);
        cout << i << endl;
    }
    return ("");
}

string webServ::POST(string requeste) {
    string respone;
    std::vector<string> body = split_string(requeste, "=&");

    data[body[1]] = body[3];
    respone = "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/plain\r\n"
              "Content-Length: 21\r\n\r\n"
              "POST request handled.";
    return respone;
}
