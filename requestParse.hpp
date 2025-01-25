#pragma once

#include <iostream>
#include <string.h>
#include <unordered_map>
#include <vector>
#include <stack>
#include <sstream>
#include <algorithm>
#include "helperFunctions/stringManipulation__.cpp"
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>  // For sockaddr_in, htons, etc.
#include <arpa/inet.h>   // For inet_pton, inet_addr, etc.
#include <sys/socket.h>  // For socket, AF_INET, etc.
#include <unistd.h>      // For close()
#define BUFFER_SIZE 8192 

using namespace std;

typedef enum e_methods{
	GET,
	POST,
	DELETE
}	t_methods;

//echo -e "GET / HTTP/1.1\r\n    Host: localhost\r\n\r\n" | nc localhost 8080 // cmd for manually writing requests

class Request {
	private:
		vector<string>									startLineComponents;
		unordered_map<string, string>					headers;
		string											body;
		stack<bool (Request::*)(stringstream&)>			parseFunctions;
		stack<void (Request::*)(const string&) const>	parseFunctionsStarterLine;
		string											remainingBuffer;
		bool											readAllRequest;

		void											isProtocole(const string& httpVersion) const;
		void											isTarget(const string& str) const;
		void											isMethod(const string& target) const;
		bool											parseStartLine(stringstream& stream);
		bool											validFieldName(string& str) const;
		bool											parseFileds(stringstream& stream);
		bool											parseBody(stringstream& stream);
		void											reconstructUri();
	public:
		Request();
		void											parseMessage(const int clientFd);
		const string&									getMethod()	const;
		const string&									getTarget()	const;
		const string&									getHttpProtocole()	const;
		const string&									getHeader(const string&);
		const bool&										getRequestStatus() const;
};
