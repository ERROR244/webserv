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
string  ft_getsockname(int clientFd);
int     ft_setsockopt(int __fd, int __level, int __optname);
int     ft_bind(int __fd, const sockaddr *__addr, socklen_t __len);
int     ft_socket(int __domain, int __type, int __protocol);
int     ft_epoll_create1(int __flags);
int     ft_listen(int __fd, int __n);
int     ft_close(int& __fd, string why);
off64_t ft_stoi(const string &__str);
off64_t my_stoi(const string &str, size_t *pos = 0, int base = 10);
