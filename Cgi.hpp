#pragma once
#include "requestParse.hpp"

#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>

class Cgi {
	private:
		unordered_map<string, string>	mapEnv;
		char**							scriptEnvp;
		char**							ncHomeEnvp;
		int								pipeToWriteToChild[2];
		int								pipeToWriteToParent[2];

		void    prepearingCgiEnvVars(Request);
		void	setupCGIProcess();
		void 	transformVectorToChar(vector<string>&);
		void	executeScript();
		void	writeToScrpit();
		void	readFromScript();
	public:
		Cgi(char**);
};