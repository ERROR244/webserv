#include <iostream>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <string.h>
#include <regex>
// #include <boost/algorithm/string.hpp>

using namespace std;
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
			parseFileds(stream);
		}


		bool	isProtocole(const string& httpVersion) {
			if (!strncmp(httpVersion.c_str(), "HTTP/", 5) || isdigit(httpVersion[5]) || httpVersion[6] == '.' || isdigit(httpVersion[7]))
				return true;
			return false;
		}

		bool isTarget(const string& target) {
			// what form is the request-target
			// reconstruct the full URI usin the components (scheme authority queries)
			string scheme;
			string authority;
			string path;
			string query;

			
			return false;
		}

		bool isMethod(const string& target) {
			return false;
		}

		void	parseStartLine(const string& startLine) {
			stringstream stream(startLine);
			vector<string> startLineComps;
			//idk how regex works
			regex delimiter("\\s+");
			sregex_token_iterator it(startLine.begin(), startLine.end(), delimiter, -1);
    		sregex_token_iterator end;

			for (; it != end; ++it) { //any white space dilimeter can be used
				startLineComps.push_back(*it);
			}
			if (startLineComps.size() > 3 || !isProtocole(startLineComps[2]) || !isTarget(startLineComps[1]) || isMethod(startLineComps[0])) {
				throw("bad request");
			}
		}


		void	parseFileds(stringstream& stream) {
			string line;
			while(getline(stream, line)) {
				vector<string> fieldsComps;
				regex delimiter("\\s+");
				sregex_token_iterator it(line.begin(), line.end(), delimiter, -1);
    			sregex_token_iterator end;
				string fieldName;
				string filedValue;

				for (; it != end; ++it) {
					fieldsComps.push_back(*it);
				}
				fieldName = fieldsComps[0].substr(0, fieldsComps[0].size()-1);
				// filedValue = trim()
			}
		}
};