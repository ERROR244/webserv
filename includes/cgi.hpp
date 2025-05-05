#pragma once
#include <map>
#include <string>
#include <vector>
#include <cerrno>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "wrappers.h"
#include "statusCodeException.hpp"

using namespace std;

#define DOCUMENT_ROOT "./www/"

struct cgiInfo {
	string	scriptUri;
	string	scriptName;
	string	exec;
	string	path;
	string	query;
	string	method;
};

class Cgi {
	private:
		const cgiInfo		infos;
		pid_t				pid;
		int					rPipe[2];
		int					wPipe[2];
		map<string, string>	scriptEnvs;

		void	createPipes();
		void	executeScript();
		void	getHeaders(const map<string, vector<string> >& headers);
		Cgi();
	public:
		Cgi(const cgiInfo& infos);
		~Cgi();
		void	setupCGIProcess();
		void    prepearingCgiEnvVars(const map<string, vector<string> >& headers);
		int		wFd();
		int		rFd();
		int		ppid();
};