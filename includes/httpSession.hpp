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
#include "binarystring.hpp"
#include "stringManipulation.h"
#include "statusCodeException.hpp"
#include <ctime>

#define BUFFER_SIZE 8192
#define T 5

using namespace std;

class httpSession {
private:
	eMethods			method;
	string				path;
	string				query;
	string				httpProtocole;
	map<string, string>	headers;
	int					statusCode;
	string				codeMeaning;
	Cgi*				cgi;
	location*			locationRules;
public:
	class Request {
	private:
		httpSession&						s;
		string								prvsFieldName;
		string								prvsContentFieldName;
		queue<bool(Request::*)(bstring&)>	parseFunctions;
		queue<bool(Request::*)(bstring&)>	bodyParseFunctions;
		map<string, string>					contentHeaders;
		int									length;
		ofstream							fd;
		string								boundaryValue;
		bstring								remainingBuffer;
		t_state								state;

		void								isCGI(location*);
		void								reconstructUri(location* rules);
		void								isProtocole(bstring& httpVersion);
		void								extractPathQuery(bstring& uri);
		void								isTarget(bstring& target);
		void								isMethod(bstring& method);
		location*							getConfigFileRules();
		bool								parseStartLine(bstring&);
		bool								validFieldName(string& str) const;
		bool								parseFileds(bstring&);
		void								openTargetFile(const string& filename, ofstream& fd) const;
		bool								boundary(bstring&);
		bool								fileHeaders(bstring&);
		bool								fileContent(bstring&);
		bool								contentLengthBased(bstring&);
		bool								transferEncodingChunkedBased(bstring&);
		bool								parseBody(bstring&);
	public:
		Request(httpSession& session);
		void								parseMessage(const int clientFd);
		const t_state&						status() const;
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
		string				getDeleteRes(const string& path, const string& connection, struct stat& file_stat);
		string				deleteDir(const string& dir, const string& connection);
		string				deleteFile(const string& file, const string& connection);
		void				sendRes(int clientFd, bool smallFile, struct stat& file_stat);
		void    			Get(int clientFd, bool smallFile);
        void    			sendBodyifChunked(int clientFd);
		string				getExt(string path);
	public:
		Response(httpSession& session);

		time_t				handelClientRes(const int clientFd);
		const t_state&		status() const;
	};

	Request		req;
	Response	res;
	configuration*		config;

	httpSession(int clientFd, configuration* confi);
	httpSession();

	void				reSetPath(const string& newPath);
	map<string, string>	getHeaders() { return headers; }

	string	sessionId;
	bool	cookieSeted;
};

bool				checkTimeOut(map<int, time_t>& timeOut, const int& clientFd, time_t lastActivityTime);
string				getSessionID(const map<string, string>& headers);
string				generateSessionID();
string				getSessionCookie(string& sessionID);
void				setCookie(string& sessionId, const string& cookieId);


