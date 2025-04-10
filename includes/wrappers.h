#pragma once
#include <iostream>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <string.h>
#include <stdexcept>
#include <climits>
#include <cctype>
#include <statusCodeException.hpp>

using namespace std;

string  w_realpath(const char * file_name);
string  toString(const int& nbr);
int     w_stoi(const string& snum);
string  getsockname(int clientFd);
int     ft_setsockopt(int __fd, int __level, int __optname);
int     ft_epoll_ctl(int __epfd, int __op, int __fd, epoll_event *event);
int     ft_bind(int __fd, const sockaddr *__addr, socklen_t __len);
int     ft_socket(int __domain, int __type, int __protocol);
// int     ft_fcntl(int __fd, int __cmd1, int __cmd2);
int     ft_epoll_create1(int __flags);
int     ft_listen(int __fd, int __n);
int     ft_close(int& __fd, string why);
int     ft_epoll__ctl(int __epfd, int __fd, epoll_event* ev);
int     ft_stoi(const std::string &__str);
int     my_stoi(const std::string &str, size_t *pos, int base);
void    ft_perror(const std::string &msg = "");
