#include "wrapperFunc.hpp"

int ft_socket(int __domain, int __type, int __protocol) {
    int fd = socket(__domain, __type, __protocol);
    if (fd == -1) {
        throw "Socket creation failed";
    }
    return fd;
}

int ft_setsockopt(int __fd, int __level, int __optname) {
    int opt = 1;
    if (setsockopt(__fd, __level, __optname, &opt, sizeof(opt)) < 0) {
        ft_close(__fd);
        throw "setsockopt failed";
    }
    return 0;
}

int ft_fcntl(int __fd, int __cmd1, int __cmd2) {
    if (fcntl(__fd, __cmd1, __cmd2) < 0) {
        ft_close(__fd);
        throw "setsockopt failed";
    }
    return 0;
}

int ft_bind(int __fd, const sockaddr *__addr, socklen_t __len) {
    if (bind(__fd, __addr, __len) < 0) {
        ft_close(__fd);
        throw "Bind failed";
    }
    return 0;
}

int ft_listen(int __fd, int __n) {
    if (listen(__fd, __n) < 0) {
        ft_close(__fd);
        throw "listen failed";
    }
    return 0;
}

int ft_close(int __fd) {
    if (close(__fd) < 0) {
        cerr << "close failed" << endl;
    }
    return 0;
}

int ft_epoll_create1(int __flags) {
    int epollFd = epoll_create1(__flags);
    if (epollFd == -1) {
        throw "epoll_create1 failed";
    }
    return epollFd;
}

int ft_epoll_ctl(int __epfd, int __op, int __fd, epoll_event *event) {
    if (epoll_ctl(__epfd, __op, __fd, event) < 0) {
        ft_close(__epfd);
        throw "Epoll ctl failed";
    }
    return 0;
}

int ft_epoll_wait(int __epfd, epoll_event *__events, int __maxevents, int __timeout, vector<int> serverFd, int epollFd) {
    int nfds;

    if ((nfds = epoll_wait(__epfd, __events, __maxevents, __timeout)) == -1) {
        for (int fd = 0; fd < (int)serverFd.size(); ++fd) {
            ft_close(serverFd[fd]);
        }
        ft_close(epollFd);
        throw "epoll_wait failed";
    }
    return nfds;
}



