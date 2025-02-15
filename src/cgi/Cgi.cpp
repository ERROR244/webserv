#include "cgi.hpp"

Cgi::Cgi(const cgiInfo& infos): infos(infos), rPipe({-1}), wPipe({-1}){
	createPipes();
}
Cgi::~Cgi() {
	cerr << "des called" << endl;
}

void	Cgi::createPipes() {
	if (pipe(wPipe) < 0 || pipe(rPipe) < 0) {
		perror("pipe failed"); throw(statusCodeException(500, "Internal Server Error"));
	}
}
















// void	Cgi::prepearingCgiEnvVars(Request req, map<string, string>& mapEnvp) {
// 	mapEnvp["GATEWAY_INTERFACE"] = "CGI/1.1";//idk
// 	mapEnvp["SERVER_PROTOCOL"] = "http/1.1";
// 	mapEnvp["SERVER_NAME"] = "localhost";
// 	mapEnvp["REMOTE_METHODE"] = req.getMethod();
// 	mapEnvp["PATH_INFO"] = path;
// 	mapEnvp["QUERY_STRING"] = query;
// 	mapEnvp["SCRIPT_NAME"] = scriptName;
// 	mapEnvp["CONTENT_LENGTH"] = req.getHeader("content-length");
// 	mapEnvp["CONTENT_TYPE"] = req.getHeader("content-type");
// 	mapEnvp["HTTP_ACCEPT"] = req.getHeader("accept");
// 	mapEnvp["HTTP_ACCEPT_CHARSET"] = req.getHeader("accept-charset");
// 	mapEnvp["HTTP_ACCEPT_ENCODING"] = req.getHeader("accept-encoding");
// 	mapEnvp["HTTP_ACCEPT_LANGUAGE"] = req.getHeader("accept-language");
// 	mapEnvp["HTTP_USER_AGENT"] = req.getHeader("user-agent");
// 	// mapEnvp["HTTP_HOST"] = "";//idk
// 	// mapEnvp["PATH_TRANSLATED"] = "";//idk
// 	// mapEnvp["REMOTE_ADDR"] = "";//idk
// 	// mapEnvp["REMOTE_HOST"] = "";
// 	// mapEnvp["REMOTE_USER"] = "";
// 	// mapEnvp["SERVER_PORT"] = "";
// 	// mapEnvp["WEBTOP_USER"] = "";
// }

static char**	transformVectorToChar(vector<string>& vec) {
	char** 	strs = new char*[vec.size() + 1];

	for (int i = 0; i < vec.size(); ++i) {
		strs[i] = new char[vec[i].size()+1];
		strcpy(strs[i], vec[i].c_str());
	}
	strs[vec.size()] = NULL;
	return (strs);
}

void	Cgi::executeScript() {
	char**							argv, CGIEnvp;
	vector<string>					vecArgv, vecEnvp;
	// map<string, string>	mapEnvp;
	// prepearingCgiEnvVars(req, mapEnvp);
	// for(const auto& it: mapEnvp) {
	// 	if (!it.second.empty())
	// 		vecEnvp.push_back(it.first + "=" + it.second);
	// }
	// while(*ncHomeEnvp) {
	// 	vecEnvp.push_back(*ncHomeEnvp);
	// 	++ncHomeEnvp;
	// }
	// CGIEnvp = transformVectorToChar(vecEnvp);
	//exec
	if (!infos.exec.empty())
		vecArgv.push_back(infos.exec);
	vecArgv.push_back(infos.scriptUri);
	argv = transformVectorToChar(vecArgv);
	if (execve(argv[0], argv, NULL) == -1) {
		for (int i = 0; argv[i]; ++i) {
			delete argv[i];
		}
		delete []argv;
		perror("execve failed(cgi.cpp 102)"); exit(-1);
	}
}

void	Cgi::setupCGIProcess() {
	// //fd[1] // write end;
	// //fd[0] // read end;
	pid_t pid = fork();
	if (pid < 0) {
		perror("fork failed"); exit(-1);
	}
	else if(pid == 0) {
		close(rPipe[0]);
		close(wPipe[1]);

		if (dup2(rPipe[1], STDOUT_FILENO) < 0 || dup2(wPipe[0], STDIN_FILENO) < 0) {
			perror("dup2 failed"); exit(-1);
		}
		close(rPipe[1]);
		close(wPipe[0]);
		executeScript();
	}
	wait(0);//use waitpid and read from the pipe to free space for the write
	close(rPipe[1]);
	close(wPipe[0]);
}