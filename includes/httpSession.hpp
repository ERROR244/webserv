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
// #include <pair>
#include <ctime>



#define BUFFER_SIZE 8192
#define URI_MAXSIZE 1024
#define HEADER_FIELD_MAXSIZE 5120
#define T 5

using namespace std;

enum e_sstat {//session stat
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
	ss_sBody,
	ss_done,
	ss_cclosedcon,
};

class httpSession {
private:
	const int			clientFd;
	e_sstat				sstat;
	e_methods			method;
	string				path;
	string				query;
	map<string, string>	headers;
	configuration		config;
	location*			rules;
	Cgi*				cgi;
	bstring				cgiBody;
	int					statusCode;
	string				codeMeaning;
	string				returnedLocation;
	bool				closeAutoIndex;

public:
	class Request {
	private:
		httpSession&	s;
		void			(httpSession::Request::*bodyHandlerFunc)(const bstring&, size_t);
		bstring			remainingBody;
		string			boundary;
		size_t			length;
		int				fd;

		int				parseStarterLine(const bstring& buffer);
		void			contentlength(const bstring&, size_t);
		void			unchunkBody(const bstring&, size_t);
		void			bufferTheBody(const bstring&, size_t);
		void			bodyFormat();
		void			isCGI();
		void			reconstructUri();
	public:
		void			readfromsock();
		Request(httpSession& session);
	};

	class Response {
	private:
		httpSession&	s;
		bool			headerSended;
		int				contentFd;
		bool			cgiHeadersParsed;
		time_t      	lastActivityTime;
		
		static string		getSupportedeExtensions(const string&);
		void				sendCgiStarterLine(const int);
		void				sendCgiOutput(const int);
		string				getDeleteRes(const string& path, const string& connection, struct stat& file_stat);
		string				deleteDir(const string& dir, const string& connection);
		string				deleteFile(const string& file, const string& connection);
		void				sendRes(int clientFd, bool smallFile, struct stat& file_stat);
		void    			Get(int clientFd, bool smallFile);
        void    			sendBodyifChunked(int clientFd);
		void				handelRedirection(const int clientFd);
		string				getExt(string path);
	public:
		Response(httpSession& session);

		time_t				handelClientRes(const int clientFd);
	};
	httpSession(int clientFd, configuration& confi);
	httpSession(const httpSession& other);
	httpSession();
	~httpSession();

	Request		req;
	Response	res;


	int					parseFields(const bstring& buffer, size_t pos, map<string, string>& headers);
	configuration		clientConfiguration() const;
	int					fd() const;
	const e_sstat&		status() const;
	void				reSetPath(const string& newPath);
	map<string, string>	getHeaders() { return headers; }
};

bool				checkTimeOut(map<int, time_t>& timeOut, const int& clientFd, time_t lastActivityTime);
string				generate_autoindex_html(const string& dir_path, const string& uri_path);
bool				write_to_file(const string& filename, const string& content);
int		    		ft_epoll_wait(int __epfd, epoll_event *__events, int __maxevents, int __timeout, map<int, httpSession> serverFd, int epollFd);
