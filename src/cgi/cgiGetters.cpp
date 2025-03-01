#include "cgi.hpp"

int Cgi::wFd() {
	return wPipe[1];
}

int	Cgi::rFd() {
	return rPipe[0];
}

int	Cgi::ppid() {
	return pid;
}