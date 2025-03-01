#include "cgi.hpp"

Cgi::Cgi(const cgiInfo& infos): infos(infos), rPipe{-1}, wPipe{-1}{
	createPipes();
}

Cgi::~Cgi() {
	cerr << "des called" << endl;
}

void	Cgi::createPipes() {
	if (pipe(wPipe) < 0 || pipe(rPipe) < 0) {
		perror("pipe failed");
		throw(statusCodeException(500, "Internal Server Error"));
	}
}

static string	proccesHeaderValue(const string& key) {
	string proccesedKey = "HTTP_";

	for (int i = 0; i < key.size(); ++i) {
		char ch = toupper(key[i]);
		if (ch == '-')
			ch = '_';
		proccesedKey += ch;
	}
	return proccesedKey;
}

void	Cgi::getHeaders(const map<string, string>& headers) {
	for (map<string, string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		scriptEnvs[proccesHeaderValue(it->first)] = it->second;
}	

void	Cgi::prepearingCgiEnvVars(const map<string, string>& headers) {
	scriptEnvs["GATEWAY_INTERFACE"] = "CGI/1.1";
	scriptEnvs["SERVER_PROTOCOL"] = "http/1.1";
	scriptEnvs["SERVER_NAME"] = "127.0.0.2";
	scriptEnvs["REMOTE_METHODE"] = "GET";
	scriptEnvs["PATH_INFO"] = infos.path;
	scriptEnvs["QUERY_STRING"] = infos.query;
	scriptEnvs["SCRIPT_NAME"] = infos.scriptName;
	scriptEnvs["SCRIPT_FILENAME"] = infos.scriptUri;
	scriptEnvs["REDIRECT_STATUS"] = "200";
	getHeaders(headers);
	// scriptEnvs["PATH_TRANSLATED"] = "";//idk
	// for (const auto& it : scriptEnvs)
	// 	cerr << it.first << ": " << it.second << endl;
}

static char**	transformVectorToChar(vector<string>& vec) {
	char** 	strs = new char*[vec.size() + 1];

	for (size_t i = 0; i < vec.size(); ++i) {
		strs[i] = new char[vec[i].size()+1];
		strcpy(strs[i], vec[i].c_str());
	}
	strs[vec.size()] = NULL;
	return strs;
}

void	Cgi::executeScript() {
	char			**CGIEnvp, **argv;
	vector<string>	vecArgv, vecEnvp;

	for(map<string, string>::iterator it = scriptEnvs.begin(); it != scriptEnvs.end(); ++it) {
		vecEnvp.push_back(it->first + "=" + it->second);
	}
	// while(*ncHomeEnvp) { // is it needed for me to add home envp
	// 	vecEnvp.push_back(*ncHomeEnvp);
	// 	++ncHomeEnvp;
	// }
	if (!infos.exec.empty())
		vecArgv.push_back(infos.exec);
	vecArgv.push_back(infos.scriptUri);
	argv = transformVectorToChar(vecArgv);
	CGIEnvp = transformVectorToChar(vecEnvp);
	execve(argv[0], argv, CGIEnvp);
	for (size_t i = 0; argv[i]; ++i) {
		delete argv[i];
	}
	delete []argv;
	for (size_t i = 0; CGIEnvp[i]; ++i) {
		delete CGIEnvp[i];
	}
	delete []CGIEnvp;
	perror("execve failed(cgi.cpp 102)");
	throw(statusCodeException(500, "Internal Server Errorrr"));
}

void	Cgi::setupCGIProcess() {
	// //fd[1] // write end;
	// //fd[0] // read end;
	pid = fork();
	if (pid < 0) {
		perror("fork failed");
		throw(statusCodeException(500, "Internal Server Error"));
	}
	else if(pid == 0) {
		close(rPipe[0]);
		close(wPipe[1]);
		if (dup2(rPipe[1], STDOUT_FILENO) < 0 || dup2(wPipe[0], STDIN_FILENO) < 0) {
			perror("dup2 failed");
			throw(statusCodeException(500, "Internal Server Error"));
		}
		close(rPipe[1]);
		close(wPipe[0]);
		executeScript();
	}
	close(rPipe[1]);
	close(wPipe[0]);
	// close(wPipe[1]);
}