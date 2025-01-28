/*
	fork and run teh script and pipe and read it and ship it
	before check if the headers are well formed and make the start-line and send the response
*/

//https://www.ibm.com/docs/en/netcoolomnibus/8.1?topic=scripts-environment-variables-in-cgi-script

#include "Cgi.hpp"

void	prepearingCgiEnvVars(Request req, unordered_map<string, string>& mapEnvp) {
	mapEnvp["GATEWAY_INTERFACE"] = "CGI/1.1";//idk
	mapEnvp["SERVER_PROTOCOL"] = "http/1.1";
	mapEnvp["SERVER_NAME"] = "localhost";
	mapEnvp["REMOTE_METHODE"] = req.getMethod();
	mapEnvp["PATH_INFO"] = req.getPath();
	mapEnvp["QUERY_STRING"] = req.getQuery();
	mapEnvp["SCRIPT_NAME"] = req.getScriptName();
	mapEnvp["CONTENT_LENGTH"] = req.getHeader("content-length");
	mapEnvp["CONTENT_TYPE"] = req.getHeader("content-type");
	mapEnvp["HTTP_ACCEPT"] = req.getHeader("accept");
	mapEnvp["HTTP_ACCEPT_CHARSET"] = req.getHeader("accept-charset");
	mapEnvp["HTTP_ACCEPT_ENCODING"] = req.getHeader("accept-encoding");
	mapEnvp["HTTP_ACCEPT_LANGUAGE"] = req.getHeader("accept-language");
	mapEnvp["HTTP_USER_AGENT"] = req.getHeader("user-agent");
	mapEnvp["HTTP_HOST"] = "";//idk
	mapEnvp["PATH_TRANSLATED"] = "";//idk
	mapEnvp["REMOTE_ADDR"] = "";//idk
	mapEnvp["REMOTE_HOST"] = "";
	mapEnvp["REMOTE_USER"] = "";
	mapEnvp["SERVER_PORT"] = "";
	mapEnvp["WEBTOP_USER"] = "";
}

void	writeBodyToCGI(int pipe, const string& buff) {
	write(pipe, buff.c_str(), BUFFER_SIZE);
}

// void	readResponseFromCGI(int pipe) {
// 	while (true) {
// 		char buff[BUFFER_SIZE+1] = {0};
// 		if(read(pipe, buff, BUFFER_SIZE) <= 0)	break;
// 		// response += buff;
// 	}
// }


char**	transformVectorToChar(vector<string>& vec) {
	char** 	CGIEnvp = new char*[vec.size() + 1];

	for (int i = 0; i < vec.size(); ++i) {
		CGIEnvp[i] = new char[vec[i].size()+1];
		strcpy(CGIEnvp[i], vec[i].c_str());
	}
	CGIEnvp[vec.size()] = NULL;
	return (CGIEnvp);
}

void	executeScript(char** ncHomeEnvp, Request& req) {
	char** 							CGIEnvp;
	vector<string> 					vecEnvp;
	unordered_map<string, string>	mapEnvp;
	prepearingCgiEnvVars(req, mapEnvp);
	for(const auto& it: mapEnvp) {
		if (!it.second.empty())
			vecEnvp.push_back(it.first + "=" + it.second);
	}
	while(*ncHomeEnvp) {
		vecEnvp.push_back(*ncHomeEnvp);
		++ncHomeEnvp;
	}
	CGIEnvp = transformVectorToChar(vecEnvp);
	//exec
	char *argv[] = {"script1.cgi", NULL};
	execve("/bin/script1.cgi", argv, CGIEnvp);
}

pair<int, int>	setupCGIProcess(char** ncHomeEnvp, Request& req) {
	int pipe1[2];//child->parent
	int pipe2[2];//parent->child //POST methode
	//fd[1] // write end;
	//fd[0] // read end;

	if (pipe(pipe1) < 0 || pipe(pipe2) < 0) {//communication channel
		perror("pipe failed"); exit(-1);
	}
	pid_t pid = fork();
	if (pid < 0) {
		perror("fork failed"); exit(-1);
	}
	else if(pid == 0) {
		close(pipe1[0]); //close read end
		close(pipe2[1]); //close write end

		if (dup2(pipe1[1], STDOUT_FILENO) < 0 || dup2(pipe2[0], STDIN_FILENO) < 0) {//write the stdout in the pipe1[1];//instead of reading from the stdin read from the pipe2[0];
			perror("dup2 failed"); exit(-1);
		}
		close(pipe1[1]);
		close(pipe2[0]);
		executeScript(ncHomeEnvp, req);
	}
	close(pipe1[1]);//close write end
	close(pipe2[0]);//close read end
	return {pipe1[0], pipe2[1]};
}