#include <iostream>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <string.h>
#include "helper_functions/string_manipulation.cpp"

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
	unordered_map<string, string>	headers;
	string							body;

	public:
		Request(string message) {
			parseMessage(message);
			// reconstruct the URI

			//DEBUG
			cerr << startLine << endl;
			for (const auto& h : headers) {
				cerr << h.first << ": " << h.second << endl;
			}
			cerr << body << endl;
			//DEBUG
		}

	private:
		void	parseMessage(const string& message) {
			string			line;
			stringstream	stream(message);

			while (getline(stream, line) && (line == "\r" || line.size() == 0)); //skiping any empty lines proceding the start-line
			parseStartLine(line);
			parseFileds(stream);
			parseBody(stream);
		}


		bool	isProtocole(const string& httpVersion) const {
			//if the client is usin https reject the request
			if (!strncmp(httpVersion.c_str(), "HTTP/", 5) && isdigit(httpVersion[5]) && httpVersion[6] == '.' && isdigit(httpVersion[7]))
				return true;
			return false;
		}

		bool isTarget(const string& str) const {
			const string	validCharachters = "-._~:/?#[]@!$&'()*+,;="; // valid charachters that can be in a target request

			if (strncmp(str.c_str(), "http", 4) && str[0] != '/')	return false; //if not origin form || absolute form
			for (const auto& c : str) {
				if (!iswalnum(c) && validCharachters.find(c) == string::npos)	return false;
			}
			return true;
 		}

		bool isMethod(const string& target) const {
			if (target == "GET" || target == "POST" || target == "DELETE") return true;
			return false;
		}

		void	parseStartLine(const string& startLine) {
			stringstream	stream(startLine);
			string			word;
			vector<string>	startLineComps;

			while (stream >> word) {
				startLineComps.push_back(word);
			}
			if (startLineComps.size() != 3 || !isProtocole(startLineComps[2]) || !isTarget(startLineComps[1]) || !isMethod(startLineComps[0])) {
				throw("bad request");
			}
			this->startLine = startLine;
		}


		bool    validFieldName(const string& str) const {
			for (const auto& c: str) {
				if (!iswalnum(c) && c != '_' && c != '-')	return false;
			}
			return true;
		}

		void	parseFileds(stringstream& stream) {
			//if line starting with /t ot /sp that means its a continuation for a line foldin;
			string	line;
			string	prvsFieldName;

			while(getline(stream, line) && line != "\r" && line.size() != 0) {
				string	fieldName;
				string	filedValue;//can be empty

				if (!headers.empty() && (line[0] == ' ' || line[0] == '\t')) { //handle for line folding
					headers[prvsFieldName] += " " + trim(line);
					continue ;
				}

				size_t colonIndex = line.find(':');
				fieldName = line.substr(0, colonIndex);
				if (colonIndex != string::npos && colonIndex+1 < line.size()) { //checking there's a value
					filedValue = line.substr(colonIndex+1);
					filedValue = trim(filedValue); //trimin any OWS
				}
				if (!validFieldName(fieldName)) { // a-z && 1-9 && -_
					throw("bad request");
				}
				headers[fieldName] = filedValue;
				prvsFieldName = fieldName;
			}
		}


		void	parseBody(stringstream& stream) {
			// amma assume that no content/transport encoding are applied on the body
			// check for transer encoding
			string	line;

			while(getline(stream, line)) {
				body += line;
				body += "\n";
			}
		}
};