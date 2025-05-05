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

#define MAX_EVENTS 64

using namespace std;

struct cgiTools {
	pid_t	pid;
	int 	readPipe;
	int		writePipe;
	cgiTools() : pid(-1), readPipe(-1), writePipe(-1) {}
};

enum fdType {
	serverSock,
	clientSock,
	cgiPipe,
};

struct epollPtr {
	int			fd; //can be sock or pipe;
	httpSession	*s; // if NULL sock else cgi
	fdType		type;
	time_t		timer;
	cgiTools	cgiInfo;
	epollPtr() : fd(-1),  s(NULL), timer(0) {}
};

typedef struct sockaddr_in t_sockaddr;
typedef map<int, t_sockaddr>::const_iterator t_sockaddr_it;

bool 				shouldStop(int i);
vector<string>		homeEnvVariables(char ** vars = NULL);
int					createSockets(map<string, configuration>& config, vector<int>& serverFds);
map<int, epollPtr>&	getEpollMonitor();
int					startEpoll(const vector<int>& serverFds);
void				errorResponse(const int epollFd, int clientFd, map<int, httpSession>& sessions, const statusCodeException& exception);
void				multiplexerSytm(const vector<int>& serverFds, const int& epollFd, map<string, configuration>& config);
bool				readCgiOutput(struct epoll_event ev);
bool    			writeBodyToCgi(struct epoll_event ev);
void				closeCgiPipes(int epollFd, int readPipe, int writePipe);