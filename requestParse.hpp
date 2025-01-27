#pragma once

#include <iostream>
#include <string.h>
#include <unordered_map>
#include <vector>
#include <stack>
#include <sstream>
#include <algorithm>
#include <unistd.h>
#include <arpa/inet.h>//for frecv

#define BUFFER_SIZE 8192 

using namespace std;

string  trim(const string& str);
vector<string>	split_ws(string& str);

pair<int, int>	setupCGIProcess(char** ncHomeEnvp, Request& req);

typedef enum e_methods{
	GET,
	POST,
	DELETE
}	t_methods;

//echo -e "GET / HTTP/1.1\r\n    Host: localhost\r\n\r\n" | nc localhost 8080 // cmd for manually writing requests

class Request {
	private:
		string									method;
		string									target;
		string									targetPath;
		string									targetQuery;
		string									httpVersion;
		unordered_map<string, string>			headers;
		string									body;
		stack<bool (Request::*)(stringstream&)>	parseFunctions;
		stack<void (Request::*)(string&)>	parseFunctionsStarterLine;
		string									remainingBuffer;
		pair<int, int>							pipes;
		bool									readAllRequest;

		void									isProtocole(string& httpVersion);
		void									isTarget(string& target);
		void									isMethod(string& method);
		bool									parseStartLine(stringstream& stream);
		bool									validFieldName(string& str) const;
		bool									parseFileds(stringstream& stream);
		bool									parseBody(stringstream& stream);
		void									reconstructAndParseUri(string& uri);
		vector<string>							splitStarterLine(string& str);
	public:
		Request();
		void									parseMessage(const int clientFd);
		const string&							getMethod()	const;
		const string&							getTarget()	const;
		const string&							getHttpProtocole()	const;
		const string							getHeader(const string&);
		const bool&								getRequestStatus() const;
};
