#include "webServ.hpp"

webServ::webServ(string av) {
    confi = confiClass(av);
    confi.parseFile();
    confi.printKeyValue();
    DOCUMENT_ROOT = "var/www";
    MAX_PAYLOAD_SIZE = 50 * 1024 * 1024;
}
webServ::~webServ() { }

void webServ::startSocket(const keyValue& kv) {
    int fd = ft_socket(kv.addInfo->ai_family, kv.addInfo->ai_socktype, kv.addInfo->ai_protocol);
    ft_setsockopt(fd, SOL_SOCKET, SO_REUSEADDR);
    ft_fcntl(fd, F_SETFL, O_NONBLOCK);

    ft_bind(fd, kv.addInfo->ai_addr, kv.addInfo->ai_addrlen);
    ft_listen(fd, 3);
    serverFd.push_back(fd);
}

// initialize 1 socket for each port
void webServ::createSockets() {
    map<string, keyValue>::iterator it;

    extensions = getSupportedeExtensions();
    for (it = confi.kValue.begin(); it != confi.kValue.end(); ++it) {
        startSocket(it->second);
    }
}

// startEpoll and add server sockets to the epoll instance
void webServ::startEpoll() {
    epollFd = ft_epoll_create1(0);

    struct epoll_event event = {};
    for (size_t fd = 0; fd < serverFd.size(); ++fd) {
        event.events = EPOLLIN;
        event.data.fd = serverFd[fd];
        ft_epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd[fd], &event);
    }
}

void webServ::reqResp() {
    int nfds;

    cout << "Server listening on port XX..." << endl;
    while (true) {
        nfds = ft_epoll_wait(epollFd, events, MAX_EVENTS, 0, serverFd, epollFd);

        for (int i = 0; i < nfds; i++) {
            int clientFd = events[i].data.fd;
            if (indexMap.find(clientFd) != indexMap.end() && indexMap[clientFd].lastRes != 0 && time(nullptr) - indexMap[clientFd].lastRes > T) {
                cout << indexMap[clientFd].lastRes << endl;
                if (clientFd >= 0)
                    ft_close(clientFd, "clientFd");
            }
            else if (find(serverFd.begin(), serverFd.end(), events[i].data.fd) != serverFd.end())
                handelNewConnection(events[i].data.fd);
            else if(events[i].events & EPOLLIN) {
                indexMap[events[i].data.fd].req.parseMessage(events[i].data.fd);
                if (indexMap[events[i].data.fd].req.done == true) {
                    indexMap[events[i].data.fd].method = indexMap[events[i].data.fd].req.startLineComponents[0];
                    indexMap[events[i].data.fd].requestedFile = indexMap[events[i].data.fd].KV.roots[0].alias + indexMap[events[i].data.fd].req.startLineComponents[1];
                    cout << indexMap[events[i].data.fd].requestedFile << endl;
                    ev.events = EPOLLOUT ;
                    ev.data.fd = events[i].data.fd;
                    epoll_ctl(epollFd, EPOLL_CTL_MOD, events[i].data.fd, &ev);
                    indexMap[events[i].data.fd].req = Request();
                    size_t size = indexMap[events[i].data.fd].requestedFile.find_last_of(".");
                    if (size != string::npos) {
                        string ext = indexMap[events[i].data.fd].requestedFile.substr(size);
                        if (extensions.find(ext) != extensions.end()) {
                            fileType =  extensions[ext];
                        }
                    }
                    cout << indexMap[events[i].data.fd].requestedFile << ", " << events[i].data.fd << endl;
                    cout << "done parsing the request" << endl;
                }
            }
            else if(events[i].events & EPOLLOUT) {
                handelClientRes(events[i].data.fd);
            }
        }
    }
}

