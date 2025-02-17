#pragma once
#include <map>
#include <stack>
#include <queue>
#include <vector>
#include "cgi.hpp"
#include <fcntl.h>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <algorithm>
#include <sys/stat.h>
#include "wrappers.h"
#include "confiClass.hpp"
#include "statusCodeException.hpp"
#include <ctime>

#define BUFFER_SIZE 8192
#define T 5

using namespace std;

class httpSession {
private:
	string				method;
	string				path;
	string				query;
	string				httpProtocole;
	map<string, string>	headers;
	int					statusCode;
	string				codeMeaning;
	Cgi*				cgi;
public:
	class Request {
	private:
		httpSession&	s;
		string									prvsFieldName;
		string									prvsContentFieldName;
		queue<bool(Request::*)(stringstream&)>	parseFunctions;
		queue<bool(Request::*)(stringstream&)>	bodyParseFunctions;
		map<string, string>						contentHeaders;
		int										length;
		int										fd;
		string									remainingBuffer;
		t_state									state;

		void									isProtocole(string& httpVersion);
		void									isCGI(location*);
		void									reconstructUri(location* rules);
		void									extractPathQuery(string& uri);
		void									isTarget(string& target);
		void									isMethod(string& method);
		location*								getConfigFileRules();
		bool									parseStartLine(stringstream&);
		bool									validFieldName(string& str) const;
		bool									parseFileds(stringstream&);
		int										openTargetFile() const;
		bool									contentLengthBased(stringstream&);
		bool									transferEncodingChunkedBased(stringstream&);
		bool									parseBody(stringstream&);
	public:
		Request(httpSession& session);
		void									parseMessage(const int clientFd);
		const t_state&							status() const;
	};

	class Response {
	private:
		int				contentFd;
		bool			headerSended;
		time_t      	lastActivityTime;
		t_state			state;
		httpSession&	s;
		
		static string		getSupportedeExtensions(const string&);
		void				sendCgiStarterLine(const int);
		void				sendCgiOutput(const int);
		void				sendRes(int clientFd, bool smallFile, struct stat file_stat);
		void    			GET(int clientFd, bool smallFile);
        void    			sendBodyifChunked(int clientFd);
		string				getExt(string path);
	public:
		Response(httpSession& session);

		time_t				handelClientRes(const int clientFd);
		void				setStatus();
		const t_state&		status() const;
	};

	Request		req;
	Response	res;
	configuration*	config;

	httpSession(int clientFd, configuration* confi);
	httpSession();

	void		reSetPath(const string& newPath);

};

void	checkTimeOut(map<int, time_t>& timeOut, const int& clientFd, time_t lastActivityTime);
