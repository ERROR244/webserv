#include "requestParse.hpp"

#include <stdlib.h>
#include <fcntl.h>

class Cgi {
	private:
		unordered_map<string, string>	mapEnv;
		char**							scriptEnvp;
		char**							shellEnvp;

		void    prepearingCgiEnvVars(Request);
		void	setupCGIProcess();
		void 	transformVectorToChar(vector<string>&);
		void	executeScript();
	public:
		Cgi(char** shellEnvp);
};