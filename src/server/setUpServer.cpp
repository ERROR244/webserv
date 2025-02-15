#include "confiClass.hpp"
#include "wrappers.h"

int startEpoll(const vector<int>& serverFds) {
    int epollFd = ft_epoll_create1(0);

    struct epoll_event event = {};
    for (size_t fd = 0; fd < serverFds.size(); ++fd) {
        event.events = EPOLLIN;
        event.data.fd = serverFds[fd];
        ft_epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFds[fd], &event);
    }
    cerr << "here: "  << epollFd << endl;
    return epollFd;
}

int	startSocket(const configuration& kv) {
    int fd = ft_socket(kv.addInfo->ai_family, kv.addInfo->ai_socktype, kv.addInfo->ai_protocol);
    ft_setsockopt(fd, SOL_SOCKET, SO_REUSEADDR);
    // ft_fcntl(fd, F_SETFL, O_NONBLOCK);

    ft_bind(fd, kv.addInfo->ai_addr, kv.addInfo->ai_addrlen);
    ft_listen(fd, 3);
    return fd;
}

int	createSockets(map<string, configuration>& config, vector<int>& serverFds) {
    map<string, configuration>::iterator it;
    
    for (it = config.begin(); it != config.end(); ++it) {
        int s = startSocket(it->second);
        serverFds.push_back(s);
        cout << "----> " << getsockname(s) << endl;
    }
    return startEpoll(serverFds);
}
