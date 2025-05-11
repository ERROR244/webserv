#include "cgi.hpp"

Cgi::Cgi() {}

Cgi::Cgi(const cgiInfo& infos): infos(infos){
	rPipe[0] = -1;
	rPipe[1] = -1;
	wPipe[0] = -1;
	wPipe[1] = -1;
	createPipes();
}

Cgi::~Cgi() {}

void	Cgi::createPipes() {
	if (pipe(wPipe) < 0 || pipe(rPipe) < 0) {
		cerr << "pipe failed" << endl;
		throw(statusCodeException(500, "Internal Server Error"));
	}
}

static string	proccesHeaderValue(const string& key) {
	string proccesedKey = "HTTP_";

	for (size_t i = 0; i < key.size(); ++i) {
		char ch = toupper(key[i]);
		if (ch == '-')
			ch = '_';
		proccesedKey += ch;
	}
	return proccesedKey;
}

void	Cgi::getHeaders(const map<string, vector<string> >& headers) {
	for (map<string, vector<string> >::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		string key = proccesHeaderValue(it->first);
		if (key == "HTTP_CONTENT_LENGTH" || key == "HTTP_CONTENT_TYPE")
			key.erase(key.begin(), key.begin()+5);
		scriptEnvs[key] = it->second[0];
	}
}	

void	Cgi::prepearingCgiEnvVars(const map<string, vector<string> >& headers) {
	char absolutePath[1024];

	scriptEnvs["GATEWAY_INTERFACE"] = "CGI/1.1";
	scriptEnvs["SERVER_PROTOCOL"] = "http/1.1";
	scriptEnvs["SERVER_NAME"] = "bngn/1";
	scriptEnvs["REQUEST_METHODE"] = infos.method;
	scriptEnvs["PATH_INFO"] = infos.path;
	scriptEnvs["QUERY_STRING"] = infos.query;
	scriptEnvs["SCRIPT_NAME"] = infos.scriptName;
	scriptEnvs["SCRIPT_FILENAME"] = infos.scriptUri;
	/*
		this is just a work around to make php work because it needs
		the request to be redirected to it and not passed to it directly so when i set the var to 1
		its like i am sayin yas this request is bein redirected to you just execute it
	*/
	scriptEnvs["REDIRECT_STATUS"] = "1";
	getHeaders(headers);
	if (realpath((DOCUMENT_ROOT+infos.path).c_str(), absolutePath) == NULL || infos.path.empty())
		scriptEnvs["PATH_TRANSLATED"] = "";
	else
		scriptEnvs["PATH_TRANSLATED"] = string(absolutePath);
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

vector<string> homeEnvVariables(char **vars = NULL) {
	static vector<string> envp;

	if (envp.empty() && vars) {
		for (int i = 0;  vars[i]; ++i) {
			envp.push_back(vars[i]);
		}
	}
	return envp;
}

void	Cgi::executeScript() {
	char			**CGIEnvp, **argv;
	vector<string>	vecArgv, vecEnvp(homeEnvVariables());

	for(map<string, string>::iterator it = scriptEnvs.begin(); it != scriptEnvs.end(); ++it) {
		vecEnvp.push_back(it->first + "=" + it->second);
	}
	if (!infos.exec.empty())
		vecArgv.push_back(infos.exec);
	vecArgv.push_back(infos.scriptUri);
	argv = transformVectorToChar(vecArgv);
	CGIEnvp = transformVectorToChar(vecEnvp);
	execve(argv[0], argv, CGIEnvp);
	for (size_t i = 0; argv[i]; ++i) {
		delete[] argv[i];
	}
	delete []argv;
	for (size_t i = 0; CGIEnvp[i]; ++i) {
		delete[] CGIEnvp[i];
	}
	delete []CGIEnvp;
	cerr << "execve failed" << endl;
	exit(-1);
}

void	Cgi::setupCGIProcess() {
	pid = fork();
	if (pid < 0) {
		cerr << "fork failed" << endl;
		throw(statusCodeException(500, "Internal Server Error"));
	}
	else if(pid == 0) {
		close(rPipe[0]);
		close(wPipe[1]);
		if (dup2(rPipe[1], STDOUT_FILENO) < 0 || dup2(wPipe[0], STDIN_FILENO) < 0) {
			cerr << "dup2 failed" << endl;
			exit(-1);
		}
		close(rPipe[1]);
		close(wPipe[0]);
		executeScript();
	}
	close(rPipe[1]);
	close(wPipe[0]);
}