#include <iostream>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <string.h>
#include "helper_functions/stringManipulation.cpp"

using namespace std;

// typedef enum e_methods {
// 	GET,
// 	POST,
// 	DELETE
// } t_methods;
//echo -e "GET / HTTP/1.1\r\n    Host: localhost\r\n\r\n" | nc localhost 8080 // cmd for manually writing requests
/*
	- any bare CR not followed by LF should be considerd invalid or repalced with SP.
	- if whitespace if found in between the start-line and the first field it can either be ignored or removed and procede with your parsin.
	- in older HTTP/1.0 agents used to send and extra CRLF after a POST request to workaround some issue but in the HTTP/1.1 that is
		unnecessary and problematic and if the agent want to include it he should count it with the content-lenght
	- if any of the message sent doesn't follow the HTTP grammar should be rejected and the server respond with 400 (bad request) and close the connection.

	- any request should have a host field and if the the URI has an authority component than the host field should send a filed tht is identical to the one
		in the authority component
	- server MUST respond with a 400(bad request) if the host field is not found or the there's multiple host fields
*/
class Request {
	string							startLine;
	string							method;
	string							target;
	string							httpVersion;
	unordered_map<string, string>	headers;
	string							body;

	public:
		Request(string message);

	private:
		void	parseMessage(const string& message);

		bool	isProtocole(const string& httpVersion) const;
		bool	isTarget(const string& str) const;
		bool	isMethod(const string& target) const;
		void	parseStartLine(const string& startLine);

		bool    validFieldName(const string& str) const;
		void	parseFileds(stringstream& stream);

		void	parseBody(stringstream& stream);

		void	reconstructUri();
};