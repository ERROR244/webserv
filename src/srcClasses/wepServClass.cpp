#include "webServ.hpp"

webServ::webServ(string av) {
    confi = confiClass(av);
    confi.parseFile();
    DOCUMENT_ROOT = "var/www";
    cout << confi.kValue[0].bodySize << endl;
    MAX_PAYLOAD_SIZE = confi.kValue[0].bodySize * 1024 * 1024;
}
webServ::~webServ() { }

vector<int> webServ::getPorts() {
    vector<int> ports;
    ports.push_back(8080);
    ports.push_back(9090);
    return (ports);
}

void webServ::startSocket(const int& port) {
    int fd = ft_socket(AF_INET, SOCK_STREAM, 0); // create a socket
    ft_setsockopt(fd, SOL_SOCKET, SO_REUSEADDR); // configure socket options, see man setsockopt
    ft_fcntl(fd, F_SETFL, O_NONBLOCK);

    struct sockaddr_in address = {};
    address.sin_family = AF_INET;          // AF_INET = ipv4
    address.sin_addr.s_addr = INADDR_ANY;  // INADDR_ANY = listen on all network interfaces
    address.sin_port = htons(port);        // convert port to network byte order
    
    ft_bind(fd, (struct sockaddr*)&address, sizeof(address));   // bind socket to its corresponding port.
    ft_listen(fd, 3);                                           // the server waits for someone to connect (like a browser).
    serverFd.push_back(fd);                                     // add that socket the serverFd
}

// initialize 1 socket for each port
void webServ::createSockets() {
    extensions = getSupportedeExtensions();
    vector<int> ports = getPorts();
    for (int port = 0; port < (int)ports.size(); ++port) {
        startSocket(ports[port]);
    }

}

// startEpoll and add server sockets to the epoll instance
void webServ::startEpoll() {
    epollFd = ft_epoll_create1(0);

    struct epoll_event event = {};
    for (int fd = 0; fd < (int)serverFd.size(); ++fd) {
        event.events = EPOLLIN;
        event.data.fd = serverFd[fd];
        ft_epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd[fd], &event);
    }
}

void webServ::reqResp() {
    int nfds;

    cout << "Server listening on port XX..." << endl;
    while (true) {
        // wait for events on the monitored sockets
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
                    indexMap[events[i].data.fd].requestedFile = DOCUMENT_ROOT + indexMap[events[i].data.fd].req.startLineComponents[1];
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

