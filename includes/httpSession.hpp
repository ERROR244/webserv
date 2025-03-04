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
#define URI_MAXSIZE 1024
#define HEADER_FIELD_MAXSIZE 5120
#define T 5

using namespace std;

enum e_sstat {//session stat
	method=0,
	uri,
	httpversion,
	starterlineNl,
	fieldLine,
	wssBeforeFieldName,
	filedName,
	fieldNl,
	emptyline,
	bodyFormat,
	contentLengthBased,
	transferEncodingChunkedBased,
	sHeader,
	sBody,
	done,
	cclosedcon,
};

enum e_requestStat {
	headers,
	body,
};

class httpSession {
private:
	e_sstat				sstat;
	eMethods			method;
	string				path;
	string				query;
	string				httpProtocole;
	map<string, string>	headers;
	int					statusCode;
	string				codeMeaning;
	string				returnedLocation;
	Cgi*				cgi;
	location*			rules;
public:
class Request {
	private:
		httpSession&	s;
		e_requestStat	requestStat;
		bstring			remainingBody;
		string			boundary;
		size_t			length;
		int				fd;

		int				parseStarterLine(const bstring& buffer);
		int				parseFields(const bstring& buffer, size_t pos, map<string, string>& headers);
		void			parseBody(const bstring& buffer, size_t pos);
		void			isCGI();
		void			reconstructUri();
	public:
		void			readfromsock(const int clientFd);
		Request(httpSession& session);
	};

	class Response {
	private:
		int				contentFd;
		bool			headerSended;
		time_t      	lastActivityTime;
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
	};

	Request		req;
	Response	res;
	configuration*		config;

	httpSession(int clientFd, configuration* confi);
	httpSession();

	const e_sstat&	status() const;
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


