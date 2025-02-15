#pragma once
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <stdlib.h>
#include <iostream>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "confiClass.hpp"
#include "httpSession.hpp"
#include <statusCodeException.hpp>

#define MAX_EVENTS 10

using namespace std;

typedef struct sockaddr_in t_sockaddr;
typedef map<int, t_sockaddr>::const_iterator t_sockaddr_it;

int	    startSocket(const configuration& kv);
void    createSockets(std::vector<int>&serverFds, map<string, configuration>& confi);
int	    startEpoll(std::vector<int>&serverFds);
void    multiplexerSytm(vector<int>& servrSocks, const int& epollFd, map<string, configuration>& config);
void    handelNewConnection(int eventFd, int epollFds);
