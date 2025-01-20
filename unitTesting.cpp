#include <iostream>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <string.h>
#include "helperFunctions/stringManipulation__.cpp"
#include <algorithm>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>  // For sockaddr_in, htons, etc.
#include <arpa/inet.h>   // For inet_pton, inet_addr, etc.
#include <sys/socket.h>  // For socket, AF_INET, etc.
#include <unistd.h>      // For close()
#define BUFFER_SIZE 8192 

string remainingBuffer;

bool	isProtocole(const string& httpVersion) {
	//if the client is usin https reject the request
	if (!strncmp(httpVersion.c_str(), "HTTP/", 5) && isdigit(httpVersion[5]) && httpVersion[6] == '.' && isdigit(httpVersion[7]))
		return true;
	return false;
}

bool	isTarget(const string& str) {
	const string	validCharachters = "-._~:/?#[]@!$&'()*+,;="; // valid charachters that can be in a target reques

	if (strncmp(str.c_str(), "http://", 7) && str[0] != '/')	return false; //if not origin form || absolute form
	for (const auto& c : str) {
		if (!iswalnum(c) && validCharachters.find(c) == string::npos)	return false;
	}
	return true;
}

bool	isMethod(const string& target) {
	if (target == "GET" || target == "POST" || target == "DELETE") return true;
	return false;
}

void	parseStartLine(stringstream& stream) {
	static int					startLineParseProgress;
	bool						lineEndedWithLF = false;
	string						line;
	vector<string>				startLineComps;
	vector<string>::iterator	startLineCompsIt;

	getline(stream, line);
	if (!stream.eof()) {
		line += '\n';
		lineEndedWithLF = true;
	}
	startLineComps = split_ws(line);
	if (startLineComps.empty()) return ;
	if (!lineEndedWithLF)	remainingBuffer = line; // adding the left overs if the line didn't end with LF
	// cerr << remainingBuffer << endl;
	startLineCompsIt = startLineComps.begin();
	switch (startLineParseProgress)
	{
	case 0: {
		isMethod(*startLineCompsIt);
		// method = *startLineCompsIt;
		cerr << "m:  " << *startLineCompsIt << endl;
		++startLineParseProgress;
		if (++startLineCompsIt == startLineComps.end())	break;
	}
	case 1: {
		isTarget(*startLineCompsIt);
		// target = *startLineCompsIt;
		cerr << "t:  " << *startLineCompsIt << endl;
		++startLineParseProgress;
		if (++startLineCompsIt == startLineComps.end())	break;
	}
	case 2: {
		isProtocole(*startLineCompsIt);
		// httpVersion = *startLineCompsIt;
		cerr << "p:  " << *startLineCompsIt << endl;
		++startLineParseProgress;
	}
	case 3: {
		if (++startLineCompsIt != startLineComps.end())	cerr << "bad request" << endl;
	}
	}
	if ((lineEndedWithLF && startLineParseProgress != 3))
		cerr << "bad request" << endl;
	// ++parseProgress;
}

int	main() {
	string a = "G";
	remainingBuffer += a;
	stringstream streamA(remainingBuffer);
	parseStartLine(streamA);
	string b = "ET /test/test1kaf HTTP/1.1\n";
	remainingBuffer += b;
	stringstream streamB(remainingBuffer);
	parseStartLine(streamB);
}

/*
	filedName: h
	hhhhh\n
*/