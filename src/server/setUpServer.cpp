#include "confiClass.hpp"
#include "wrappers.h"
#include "server.h"

map<int, epollPtr>&	getEpollMonitor() {
    static map<int, epollPtr>   monitor;
    return monitor;
}

int startEpoll(const vector<int>& serverFds) {
    struct epoll_event  event;;
    int                 epollFd = ft_epoll_create1(0);
    map<int, epollPtr>& monitor = getEpollMonitor();

    for (size_t fd = 0; fd < serverFds.size(); ++fd) {
        event.events = EPOLLIN;
        monitor[serverFds[fd]].fd = serverFds[fd];
        event.data.ptr = &monitor[serverFds[fd]];
        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFds[fd], &event) == -1) {
            close(epollFd);
		    throw runtime_error("epoll_ctl failed");
            //close all the previouse sockets
        }
        cerr << "adding server fd -> " << serverFds[fd] << endl;
    }
    return epollFd;
}

int	startSocket(const configuration& kv) {
    int fd = ft_socket(kv.addInfo->ai_family, kv.addInfo->ai_socktype, kv.addInfo->ai_protocol);
    ft_setsockopt(fd, SOL_SOCKET, SO_REUSEADDR);

    ft_bind(fd, kv.addInfo->ai_addr, kv.addInfo->ai_addrlen);
    ft_listen(fd, 3);
    return fd;
}

int	createSockets(map<string, configuration>& config, vector<int>& serverFds) {
    map<string, configuration>::iterator it;
    
    for (it = config.begin(); it != config.end(); ++it) {
        int s = startSocket(it->second);
        serverFds.push_back(s);
    }
    return startEpoll(serverFds);
}
