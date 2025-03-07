#pragma once
#include <map>
#include <string>
#include <vector>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "statusCodeException.hpp"

using namespace std;

//https://www.ibm.com/docs/en/netcoolomnibus/8.1?topic=scripts-environment-variables-in-cgi-script

struct cgiInfo {
	string	scriptUri;
	string	scriptName;
	string	exec;
	string	path;
	string	query;
};

class Cgi {
	private:
		const cgiInfo		infos;
		pid_t				pid;
		int					wPipe[2];
		int					rPipe[2];
		map<string, string>	scriptEnvs;

		void	createPipes();
		void	executeScript();
		void	getHeaders(const map<string, string>& headers);
		public:
		Cgi(const cgiInfo& infos);
		~Cgi();
		void	setupCGIProcess();
		void    prepearingCgiEnvVars(const map<string, string>& headers);
		int		wFd();
		int		rFd();
		int		ppid();
};