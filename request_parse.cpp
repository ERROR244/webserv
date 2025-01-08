#include <iostream>
#include <unordered_map>
#include <sstream>

using namespace std;
/*
	- any bare CR not followed by LF should be considerd invalid or repalced with SP.
	- if whitespace if found in between the start-line and the first field it can either be ignored or removed and procede with your parsin.
	- in older HTTP/1.0 agents used to send and extra CRLF after a POST request to workaround some issue but in the HTTP/1.1 that is
		unnecessary and problematic and if the agent want to include it he should count it with the content-lenght
	- if any of the message sent doesn't follow the HTTP grammar should be rejected and the server respond with 400 (bad request) and close the connection.
*/
class Request {
	string startLine;
	unordered_map<string, string> headers;
	string body;
	public:
		Request(string message) {
			parseMessage(message);
		}
	private:
		void	parseMessage(const string& message) {
			string line;
			stringstream stream(message);
			while (getline(stream, line) && (line == "\r\n" || line == "\n")); //skiping any empty lines proceding the start-line
			startLine = line;
			parseStartLine(startLine);
		}

		void	parseStartLine(const string& startLine) {

		}
};