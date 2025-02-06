#include "webServ.hpp"

string getsockname(int clientFd) {
    struct sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    string res;

    if (getsockname(clientFd, (struct sockaddr*)&addr, &addrLen) == 0) {
        res = string(inet_ntoa(addr.sin_addr)) + ":" + toString(ntohs(addr.sin_port));
        return res;
    } else {
        throw "getsockname failed";
    }
}

void webServ::handelNewConnection(int eventFd) {
    int clientFd;

    if ((clientFd = accept(eventFd, NULL, NULL)) == -1) {
        cerr << "Accept failed" << endl;
        return ;
    }
    cout << "\n--------------------------------------New client connected!--------------------------------------\n" << endl;

    // set the client socket to non-blocking mode
    if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0) {
        cerr << "Failed to set non-blocking" << endl;
        ft_close(clientFd, "clientFd");
        return ;
    }

    // add the new client socket to epoll
    ev.events = EPOLLIN;
    ev.data.fd = clientFd;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &ev) == -1) {
        cerr << "epoll_ctl failed for client socket" << clientFd << endl;
        ft_close(clientFd, "clientFd");
        return ;
    }
    indexMap[clientFd].KV = confi.kValue[getsockname(clientFd)];
    indexMap[clientFd].headerSended = false;
    indexMap[clientFd].clientFd = clientFd;
    indexMap[clientFd].fileFd = -1;
    indexMap[clientFd].lastRes = 0;
}

void webServ::handelClientRes(int clientFd) {
    struct stat file_stat;
    
    if (file_stat.st_size < 10000) {
        sendRes(clientFd, true, file_stat);
    }
    else {
        sendRes(clientFd, false, file_stat);
    }
}

void webServ::sendRes(int clientFd, bool smallFile, struct stat file_stat) {
    if (indexMap[clientFd].method == "GET") {
        if (indexMap[clientFd].headerSended == false) {
            GET(clientFd, smallFile);
        }
        else {
            sendBodyifChunked(clientFd);
        }
    }
    if (indexMap[clientFd].method == "POST") {
        cout << "POST method called\n";
    }
    if (indexMap[clientFd].method == "DELETE") {
        string response;
        response = "HTTP/1.1 204 No Content\r\nContent-Type: text/html\r\nConnection: keep-alive\r\n\r\n";
        cout << "DELETE method called\n";
        if (indexMap[clientFd].requestedFile.find("var/www/uploads") != 0) {
            std::cerr << "Directory Traversal Attempt While Deleting.\n";
            response = "HTTP/1.1 403 Forbidden\r\nContent-Type: text/html\r\ncontent-length: 0\r\nConnection: keep-alive\r\n\r\n";
        }
        else if (S_ISDIR(file_stat.st_mode) != 0) {
            if (remove(indexMap[clientFd].requestedFile.c_str()) != 0) {
                std::cerr << "Deleting Non-Empty Directory.\n";
                response = "HTTP/1.1 409 Conflict\r\nContent-Type: text/html\r\ncontent-length: 0\r\nConnection: keep-alive\r\n\r\n";
            }
        }
        else if (S_ISREG(file_stat.st_mode) != 0) {
            unlink(indexMap[clientFd].requestedFile.c_str());
        }
        cout << response << endl;
        send(clientFd, response.c_str(), response.size(), MSG_DONTWAIT);
        ev.events = EPOLLIN ;
        ev.data.fd = clientFd;
        epoll_ctl(epollFd, EPOLL_CTL_MOD, clientFd, &ev);
        indexMap[clientFd].lastRes = time(nullptr);
    }
}




