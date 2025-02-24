#include "httpSession.hpp"

bool	httpSession::Request::boundary(bstring& buffer) {
	bstring	line;

	if (buffer.getheaderline(line) && line.cppstring() == boundaryValue)
		return true;
	remainingBuffer = line;
	return false;
}

bool	httpSession::Request::fileHeaders(bstring& buffer) {
	bstring	line;
	bool	eof;

	while((eof = buffer.getheaderline(line)) && !line.empty()) {
		string	fieldName;
		string	filedValue;

		if (!contentHeaders.empty() && (line[0] == ' ' || line[0] == '\t')) {
			contentHeaders[prvsContentFieldName] += " " + trim(line.cppstring());
			continue ;
		}
		size_t colonIndex = line.find(":");
		fieldName = line.substr(0, colonIndex).cppstring();
		if (colonIndex != string::npos && colonIndex+1 < line.size()) {
			filedValue = line.substr(colonIndex+1).cppstring();
			filedValue = trim(filedValue);
		}
		if (colonIndex == string::npos || !validFieldName(fieldName))
			throw(statusCodeException(400, "Bad Request"));
		contentHeaders[fieldName] = filedValue;
		prvsContentFieldName = fieldName;
	}
	if (!eof) {
		remainingBuffer = line;
		return false;
	}
	return true;
}

static string	retrieveFilename(const string& value) {
	vector<string>	fieldValueparts;
	split(value, ';', fieldValueparts);
	if (fieldValueparts.size() != 3 || strncmp("form-data" ,trim(fieldValueparts[0]).c_str(), 9))
		throw(statusCodeException(501, "Not Implemented"));
	vector<string> keyvalue;
	split(trim(fieldValueparts[1]), '=', keyvalue);
	if (keyvalue.size() != 2 || strncmp("name", keyvalue[0].c_str(), 4) || keyvalue[1][0] != '"' || keyvalue[1][keyvalue[1].size()-1] != '"')
		throw(statusCodeException(501, "Not Implemented"));
	keyvalue.clear();
	split(trim(fieldValueparts[2]), '=', keyvalue);
	if (keyvalue.size() != 2 || strncmp("filename", keyvalue[0].c_str(), 8) || keyvalue[1][0] != '"' || keyvalue[1][keyvalue[1].size()-1] != '"')
		throw(statusCodeException(501, "Not Implemented"));
	keyvalue[1].erase(keyvalue[1].begin());
	keyvalue[1].erase(keyvalue[1].end()-1);
	return keyvalue[1];
}

void	httpSession::Request::openTargetFile(const string& filename, ofstream& fd) const {
	// if (s.cgi != NULL)
	// 	fd = s.cgi->wFd();
	fd.open(filename.c_str(), ios::binary);
	if (!fd) {
		perror("open failed"); throw(statusCodeException(500, "Internal Server Error"));
    }
}

bool	httpSession::Request::fileContent(bstring& buffer) {
	size_t	endboundarypos;
	size_t	startboundarypos;
	size_t	newlinepos;
	bstring	subbuffer;
	//when manually testing segvs can occure as i assume the bondaries are structered correctly

	if (!fd.is_open()) {
		if (contentHeaders.find("content-disposition") != contentHeaders.end()) {
			string filename = retrieveFilename(contentHeaders["content-disposition"]);
			openTargetFile(s.path + "/" + filename, fd);
		} else
			throw(statusCodeException(501, "Not Implemented"));
	}
	startboundarypos = buffer.find(boundaryValue.c_str());
	endboundarypos = buffer.find((boundaryValue+"--").c_str());
	if (startboundarypos < endboundarypos) {
		cerr << "found the mid boundary" << endl;
		remainingBuffer = buffer.substr(startboundarypos, string::npos);
		// cerr << "s-------remaining" << endl;
		// cerr << remainingBuffer << endl;
		// cerr << "e-------remaining" << endl;
		--startboundarypos;//newline;
		if (buffer[startboundarypos-1] == '\r')
			--startboundarypos;
		fd.write(buffer.c_str(), startboundarypos);
		fd.close();
		bodyParseFunctions.push(&Request::boundary);
		bodyParseFunctions.push(&Request::fileHeaders);
		bodyParseFunctions.push(&Request::fileContent);
		buffer = remainingBuffer;
		remainingBuffer = NULL;
		return true;
	} else if (endboundarypos <= startboundarypos && endboundarypos != string::npos) {
		--endboundarypos;//newline;
		if (buffer[endboundarypos-1] == '\r')
			--endboundarypos;
		cerr << "found the end boundary" << endl;
		buffer.erase(endboundarypos, string::npos);
		fd.write(buffer.c_str(), buffer.size());
		fd.close();
		return true;
	} else if ((newlinepos = buffer.rfind('\n')) != string::npos && !(subbuffer = buffer.substr(newlinepos+1)).ncmp(boundaryValue.c_str(), subbuffer.size())) {
		//binary'\n'boundary
		cerr << "found potentiel unfinished boundary" << endl;
		if (buffer[newlinepos-1] == '\r')
			--newlinepos;
		fd.write(buffer.c_str(), newlinepos);
		remainingBuffer = buffer.substr(newlinepos, string::npos);
	}
		fd.write(buffer.c_str(), buffer.size());
	return false;
}

bool	httpSession::Request::contentLengthBased(bstring& buffer) {
	if (!length) {
		try {
			length = stoi(s.headers["content-length"]);
		} catch(...) {
			perror("unvalid number in content length");
			throw(statusCodeException(400, "Bad Request"));
		}
	}
	cerr << "before-length: " << length << endl;
	// cerr << "---s-content---" << endl;
	// cerr << buffer << endl;
	// cerr << "---e-content---" << endl;
	int buffersize = buffer.size();
	cerr << "buffer size: " << buffersize << endl; //weird shit here
	while(!bodyParseFunctions.empty()) {
		const auto& func = bodyParseFunctions.front();
		if (!(this->*func)(buffer))	break;
		bodyParseFunctions.pop();
	}
	length -= buffersize - remainingBuffer.size();
	cerr << "after-length: " << length << endl;
	return (length == 0) ? true : false;
}

// bool	httpSession::Request::transferEncodingChunkedBased(stringstream& stream) {
// 	if (fd == -1)
// 		fd = openTargetFile("test");
// 	while (1) {
// 		string	line;
// 		if (length <= 0) {
// 			if (getline(stream, line)) {
// 				if (stream.eof()) {
// 					remainingBuffer = line;
// 					return false;
// 				}
// 				perror("getline failed"); throw(statusCodeException(500, "Internal Server Error"));
// 			}
// 			try {
// 				length = stoi(line);
// 			}
// 			catch(...) {
// 				perror("unvalid number in chunked length"); throw(statusCodeException(500, "Internal Server Error"));
// 			}
// 			if (line == "0") {
// 				close(fd);
// 				return true;
// 			}
// 		}
// 		char buff[length+1] = {0};
// 		stream.read(buff, length);
// 		if (stream.gcount() == 0) return false;
// 		length -= stream.gcount();
// 		w_write(fd, buff, stream.gcount());
// 		getline(stream, line); // consume the \n(its not included n the length)
// 	}
// }


static bool	isMultipartFormData(const string& value) {
	vector<string>	fieldValueparts;
	split(value, ';', fieldValueparts);
	if (fieldValueparts.size() != 2)
		return false;
	if (trim(fieldValueparts[0]) != "multipart/form-data")
		return false;
	if (strncmp(trim(fieldValueparts[1]).c_str(), "boundary=", 9))
		return false;
	return true;
}


bool	httpSession::Request::parseBody(bstring& buffer) {
	if (s.method != POST)
		return true;
	if (s.headers.find("content-type") != s.headers.end() && isMultipartFormData(s.headers["content-type"])) {
		boundaryValue = "--" + s.headers["content-type"].substr(s.headers["content-type"].rfind('=')+1);
		if (s.headers.find("content-length") != s.headers.end())
			parseFunctions.push(&Request::contentLengthBased);
		// else if (s.headers.find("transfer-encoding") != s.headers.end() && s.headers["transfer-encoding"] == "chunked")
		// 	parseFunctions.push(&Request::transferEncodingChunkedBased);
	}
	else
		throw(statusCodeException(501, "Not Implemented"));
	return true;
}