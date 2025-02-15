#pragma once
#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <statusCodeException.hpp>
#include <string.h>

using namespace std;

string	w_realpath(const char * file_name);
string toString(const int& nbr);
string getsockname(int clientFd);
int ft_setsockopt(int __fd, int __level, int __optname);
int ft_epoll_wait(int __epfd, epoll_event *__events, int __maxevents, int __timeout, vector<int> serverFd, int epollFd);
int ft_epoll_ctl(int __epfd, int __op, int __fd, epoll_event *event);
int ft_bind(int __fd, const sockaddr *__addr, socklen_t __len);
int ft_socket(int __domain, int __type, int __protocol);
int ft_fcntl(int __fd, int __cmd1, int __cmd2);
int ft_epoll_create1(int __flags);
int ft_listen(int __fd, int __n);
int ft_close(int& __fd, string why);