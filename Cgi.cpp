/*
	fork and run teh script and pipe and read it and ship it
	before check if the headers are well formed and make the start-line and send the response
*/

//https://www.ibm.com/docs/en/netcoolomnibus/8.1?topic=scripts-environment-variables-in-cgi-script

#include "Cgi.hpp"

Cgi::Cgi(char** shellEnvp) : shellEnvp(shellEnvp) {}

void	Cgi::prepearingCgiEnvVars(Request req) {
	mapEnv["GATEWAY_INTERFACE"] = "CGI/1.1";//idk
	mapEnv["SERVER_PROTOCOL"] = "http/1.1";
	mapEnv["SERVER_NAME"] = "localhost";
	mapEnv["REMOTE_METHODE"] = req.getMethod();
	mapEnv["CONTENT_LENGTH"] = req.getHeader("content-length");
	mapEnv["CONTENT_TYPE"] = req.getHeader("content-type");
	mapEnv["HTTP_ACCEPT"] = req.getHeader("accept");
	mapEnv["HTTP_ACCEPT_CHARSET"] = req.getHeader("accept-charset");
	mapEnv["HTTP_ACCEPT_ENCODING"] = req.getHeader("accept-encoding");
	mapEnv["HTTP_ACCEPT_LANGUAGE"] = req.getHeader("accept-language");
	mapEnv["HTTP_USER_AGENT"] = req.getHeader("user-agent");
	mapEnv["HTTP_HOST"] = "";//idk
	mapEnv["PATH_INFO"] = "";//extract it manually from the target uri
	mapEnv["PATH_TRANSLATED"] = "";//idk
	mapEnv["QUERY_STRING"] = "";//?->
	mapEnv["REMOTE_ADDR"] = "";//idk
	mapEnv["REMOTE_HOST"] = "";
	mapEnv["REMOTE_USER"] = "";
	mapEnv["SCRIPT_NAME"] = "";
	mapEnv["SERVER_PORT"] = "";
	mapEnv["WEBTOP_USER"] = "";
}

void	Cgi::setupCGIProcess() {
	int pipe1[2];//child->parent
	int pipe2[2];//parent->child //POST methode
	//fd[1] // write end;
	//fd[0] // read end;

	pipe(pipe1);pipe(pipe2);//communication channel
	if(!fork()) {
		close(pipe1[0]); //close read end
		close(pipe2[1]); //close write end

		dup2(pipe1[1], STDOUT_FILENO);//write the stdout in the pipe1[1];
		dup2(pipe2[0], STDIN_FILENO);//instead of reading from the stdin read from the pipe2[0];
		close(pipe1[1]);
		close(pipe2[0]);
		executeScript();
	} else {
		char buff[1024] = {0};
		close(pipe1[1]);//close write end
		close(pipe2[0]);//close read end
		while (read(pipe1[0], buff, 1024) >= 0) {
			
		}
		//read from pipe1[0]
		//write to pipe2[1] //in case of POST methode
		//close after finishin the operation
	}
}

void	Cgi::transformVectorToChar(vector<string>& vec) {
	scriptEnvp = new char*[vec.size() + 1];

	for (int i = 0; i < vec.size(); ++i) {
		scriptEnvp[i] = new char[vec[i].size()+1];
		strcpy(scriptEnvp[i], vec[i].c_str());
	}
	scriptEnvp[vec.size()] = NULL;
}

void	Cgi::executeScript() {
	vector<string> envs;
	for(const auto& it: mapEnv) {
		if (!it.second.empty())
			envs.push_back(it.second);
	}
	while(*shellEnvp) {
		envs.push_back(*shellEnvp);
		++shellEnvp;
	}
	transformVectorToChar(envs);
	//exec
	char *argv[] = {"script1.cgi", NULL};
	execve("/bin/script1.cgi", argv, scriptEnvp);
}