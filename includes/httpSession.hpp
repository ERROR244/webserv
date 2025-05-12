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
#include "statusCodeException.hpp"
#include <ctime>

using namespace std;

#define BUFFER_SIZE 8192
#define URI_MAXSIZE 1024
#define HEADER_FIELD_MAXSIZE 5120
#define T 2

string	getHeaderValue(map<string, vector<string> >& mp, const string& key);

enum e_sstat {
	ss_method=0,
	ss_uri,
	ss_httpversion,
	ss_starterlineNl,
	ss_fieldLine,
	ss_wssBeforeFieldName,
	ss_filedName,
	ss_fieldNl,
	ss_emptyline,
	ss_body,
	ss_sHeader,
	ss_CgiResponse,
	ss_sBody,
	ss_sBodyAutoindex,
	ss_done,
	ss_cclosedcon,
};

class httpSession {
private:
	const int						clientFd;
	e_sstat							sstat;
	e_methods						method;
	string							path;
	string							query;
	map<string, vector<string> >	headers;
	configuration					config;
	location*						rules;
	Cgi*							cgi;
	bstring							cgiBody;
	string							rawUri;
	string							returnedLocation;
	bool							showDirFiles;
	int								statusCode;
	string							codeMeaning;
public:
	class Request {
	private:
		httpSession&	s;
		void			(httpSession::Request::*bodyHandlerFunc)(const bstring&, size_t);
		bstring			remainingBody;
		string			boundary;
		off64_t			length;
		ofstream		outputFile;

		int				parseStarterLine(const bstring& buffer);
		bool			fileExistence();
		void			contentlength(const bstring&, size_t);
		void			unchunkBody(const bstring&, size_t);
		void			bufferTheBody(const bstring&, size_t);
		void			bodyFormat();
		void			isCGI();
		void			reconstructUri();
	public:
		Request(httpSession& session);
		Request(const Request& other);
		~Request();
		void			readfromsock();
	};

	class Response {
	private:
		httpSession&		s;
		ifstream			inputFile;
		bstring				cgiBuffer;
		bool				addChunkedWhenSendingCgiBody;
		bool				cgiHeadersParsed;

		static string		getSupportedeExtensions(const string&);
		void				sendCgiOutput();
		void				sendHeader();
		string				contentTypeHeader() const;
		void				sendBody();
		void				generateHtml();
		void				deleteContent();
	public:
		Response(httpSession& session);
		Response(const Response& other);
		~Response();
		void				handelClientRes(const int);
		void				storeCgiResponse(const bstring&);
	};
	Request		req;
	Response	res;

	httpSession(int clientFd, configuration& confi);
	httpSession(const httpSession& other);
	httpSession();
	~httpSession();

	int								parseFields(const bstring& buffer, size_t pos, map<string, vector<string> >& headers);
	void							resetForSendingErrorPage(const string& errorPagePath, int statusCode, string meaning);
	configuration					clientConfiguration() const;
	int								fd() const;
	const e_sstat&					status() const;
	void							closeCon();
	map<string, vector<string> >	getHeaders();
	bstring&						getCgiBody();
	Cgi* getCgi();
};