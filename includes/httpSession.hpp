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
#include "statusCodeException.hpp"

#define BUFFER_SIZE 8192

using namespace std;

// struct location {
// 	string				uri;
//     vector<string>		methods;
//     string				redirection;
//     string				alias;
// 	string				upload;
//     string				index;
// 	map<string, string>	cgi;
//     bool				autoIndex;
// 	location() : index("index.html") {}
// };

// struct configuration {
//     int						bodySize;
//     map<int, string>		errorPages;
//     map<string, location>	locations;
// };

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
	configuration*		config;
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
		// bool									boundary(stringstream&);
		// bool									fileHeaders(stringstream&);
		// bool									fileContent(stringstream&);
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
		httpSession&	s;
		int				contentFd;
		t_state			state;
		
		static string	getSupportedeExtensions(const string&);
		string			contentTypeHeader() const;
		void			sendHeader(const int);
		void			sendBody(const int);
		void			sendCgiStarterLine(const int);
		void			sendCgiOutput(const int);
	public:
		Response(httpSession& session);
		void			sendResponse(const int clientFd);
		const t_state&	status() const;
	};

	Request		req;
	Response	res;

	httpSession(int clientFd, configuration* confi);
	httpSession();

	void		reSetPath(const string& newPath);

};