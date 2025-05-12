#include "httpSession.hpp"

static void	split(const string& str, const char delimiter, vector<string>& parts) {
	int	i = 0, pos = 0;

	while (str[i]) {
		if (str[i] == delimiter) {
			parts.push_back(str.substr(pos, i - pos));
			while (str[i] && str[i] == delimiter)
				++i;
			pos = i;
		} else
			++i;
	}
	parts.push_back(str.substr(pos));
}

static off64_t w_stoi(const string& snum) {
	off64_t num;
	try {
		//it will throw incase of invalid arg
		num = my_stoi(snum);
	} catch (...) {
		cerr << "w_stoi failed" << endl;
		throw(statusCodeException(400, "Bad Request"));
	}
	return num;
}

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

static int getLastChar(bstring buffer, ssize_t boundaryStartinIndex, bool startBoundary) {
	if (startBoundary)
		return boundaryStartinIndex;
	if (boundaryStartinIndex)
		--boundaryStartinIndex;
	if (boundaryStartinIndex && buffer[boundaryStartinIndex-1] == '\r')
		--boundaryStartinIndex;
	return boundaryStartinIndex;
}

static string	getFileName(const string& value, const string& path) {
	vector<string>	fieldValueparts;
	vector<string> keyvalue;

	split(value, ';', fieldValueparts);
	if (fieldValueparts.size() != 3 || strncmp("form-data" ,trim(fieldValueparts[0]).c_str(), 9))
		throw(statusCodeException(501, "Not Implemented"));
	split(trim(fieldValueparts[1]), '=', keyvalue);
	if (keyvalue.size() != 2 || strncmp("name", keyvalue[0].c_str(), 4) || keyvalue[1][0] != '"' || keyvalue[1][keyvalue[1].size()-1] != '"')
		throw(statusCodeException(501, "Not Implemented"));
	keyvalue.clear();
	split(trim(fieldValueparts[2]), '=', keyvalue);
	if (keyvalue.size() != 2 || strncmp("filename", keyvalue[0].c_str(), 8) || keyvalue[1][0] != '"' || keyvalue[1][keyvalue[1].size()-1] != '"')
		throw(statusCodeException(501, "Not Implemented"));
	keyvalue[1].erase(keyvalue[1].begin());
	keyvalue[1].erase(keyvalue[1].end()-1);
	if (keyvalue[1].empty())
		throw(statusCodeException(422, "Unprocessable Entity"));
	return path + "/" + keyvalue[1];
}

static bool	roundedByNl(const bstring& buffer, const size_t start, const size_t len) {
	char	ch;

	if ((start && buffer[start-1] != '\n'))
		return false;
	for (size_t i = start+len; i < buffer.size(); ++i) {
		ch = buffer[i];
		switch (ch)
		{
		case '\r': {
			if (i != start+len)
				return false;
			break;
		}
		case '\n':
			return true;
		default:
			return false;
		}
	}
	return false;
}

void	httpSession::Request::contentlength(const bstring& buffer, size_t pos) {
	ssize_t	contentStartinPos = pos;

	if (length >= off64_t(buffer.size() - pos))
		length -= buffer.size() - pos;
	else
		length = 0;
	while (pos < buffer.size()) {
		size_t	boundaryStartinPos = buffer.find(boundary.c_str(), pos);
		bool	firstBoundary = (outputFile.is_open()) ? false : true;
		int		sepBoundary = 0;

		if (firstBoundary && (boundaryStartinPos == string::npos || pos != boundaryStartinPos))
			throw (statusCodeException(400, "Bad Request"));
		else if (boundaryStartinPos == string::npos)
			break;
		//checking the type of boundary;
		if(!buffer.ncmp((boundary+"--").c_str(), boundary.size()+2, boundaryStartinPos))
			sepBoundary = 2;
		//check if boundary is rounded by newlines;
		if (roundedByNl(buffer, boundaryStartinPos, boundary.size()+sepBoundary)) {
			//getting the index of the last charchter in the content of the previous file
			ssize_t lastIndexOfPContent = getLastChar(buffer, boundaryStartinPos, firstBoundary);
			if (sepBoundary == 0) {
				map<string, vector<string> >	contentHeaders;
				if (outputFile.is_open()) {
					outputFile.write(&buffer[contentStartinPos], lastIndexOfPContent-contentStartinPos);
					if (outputFile.fail())
						throw(statusCodeException(500, "Internal Server Error"));
				}
				s.sstat = ss_emptyline;
				if ((contentStartinPos = s.parseFields(buffer, buffer.find('\n', lastIndexOfPContent+boundary.size())+1, contentHeaders)) < 0) {
					remainingBody = buffer.substr(lastIndexOfPContent);
					length += remainingBody.size();
					outputFile.close();
					s.sstat = ss_body;
					return;
				}
				s.sstat = ss_body;
				outputFile.close();
				string filePath = getFileName(getHeaderValue(contentHeaders, "content-disposition"), s.rules->uploads);
				if (access(filePath.c_str(), F_OK) != -1)
					throw(statusCodeException(409, "Conflict"));
				outputFile.open(filePath.c_str());
				if (outputFile.is_open() == false)
					throw(statusCodeException(500, "Internal Server Error"));
			} else if (sepBoundary == 2) {
				if (outputFile.is_open()) {
					outputFile.write(&buffer[contentStartinPos], lastIndexOfPContent-contentStartinPos);
					if (outputFile.fail())
						throw(statusCodeException(500, "Internal Server Error"));
				}
				if (length == 0) {
					s.sstat = ss_sHeader;
					outputFile.close();
					return;
				}
				else
					throw (statusCodeException(400, "Bad Request"));
			}
		}
		pos = boundaryStartinPos+boundary.size();
	}
	if (static_cast<size_t>(contentStartinPos) >= buffer.size() || outputFile.is_open() == false)
		return;
	size_t cuttingPoint = buffer.size() > boundary.size()+4 ? (buffer.size()-1)-(boundary.size()+4) : 0;
	remainingBody = buffer.substr(cuttingPoint);
	length += remainingBody.size();
	outputFile.write(&buffer[contentStartinPos], buffer.size()-contentStartinPos-remainingBody.size());
	if (outputFile.fail())
		throw(statusCodeException(500, "Internal Server Error"));
}

void	httpSession::Request::unchunkBody(const bstring& buffer, size_t pos) {
	size_t buffersize = buffer.size();

	while (pos < buffersize) {
		stringstream	ss;
		size_t			nlPos = 0;
	
		if (length == 0) {
			nlPos = buffer.find('\n', pos);
			if (nlPos == string::npos) {
				remainingBody = buffer.substr(pos);
				return;
			} else {
				bool 	crInLine = (nlPos > 0 && buffer[nlPos-1] == '\r');
				size_t	lengthEnd = crInLine ? nlPos - 1 : nlPos;
				string	lengthInHex = buffer.substr(pos, lengthEnd-pos).cppstring();

				for (size_t i = 0; i < lengthInHex.size(); ++i) {
                    char c = lengthInHex[i];
                    if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
                        throw statusCodeException(400, "Invalid hex character in chunk size");
                    }
                }
				ss << hex << lengthInHex;
				ss >> length;
				if (length == 0) {
					s.headers["content-length"].push_back(toString(s.cgiBody.size()));
					s.headers.erase(s.headers.find("transfer-encoding"));
					s.sstat = ss_sHeader;
					return;
				}
				pos = nlPos+1;
			}
		}
		size_t chunkEnd = pos + length;
        if (chunkEnd + 2 > buffersize) {
            // We don't have the complete chunk data yet - save what we have and wait
            remainingBody = buffer.substr(pos);
            return;
        }
        // Append the chunk data to our body
        s.cgiBody += buffer.substr(pos, length);
        if (static_cast<off64_t>(s.cgiBody.size()) > s.config.bodySize) {
            throw statusCodeException(413, "Request Entity Too Large");
        }
        pos = chunkEnd;
        if (pos + 1 < buffersize && buffer[pos] == '\r' && buffer[pos + 1] == '\n') {
            pos += 2;
        } else if (pos < buffersize && buffer[pos] == '\n') {
            pos += 1;
        } else {
            throw statusCodeException(400, "Malformed chunked encoding");
		}
        // Reset for next chunk
        length = 0;
	}
}

void	httpSession::Request::bufferTheBody(const bstring& buffer, size_t pos) {
	if (length) {
		s.cgiBody += buffer.substr(pos, length);
		if (off64_t(s.cgiBody.size()) >= length)
			length = 0;
		else 
			length -= s.cgiBody.size();
	}
	if (length == 0)
		s.sstat = ss_sHeader;
}

void	httpSession::Request::bodyFormat() {
	if (s.cgi) {
		if (s.headers.find("content-length") != s.headers.end()) {
			length = w_stoi(getHeaderValue(s.headers, "content-length"));
			if (length > s.config.bodySize)
				throw(statusCodeException(413, "Request Entity Too Large"));
			bodyHandlerFunc = &Request::bufferTheBody;
		}
		else if (s.headers.find("transfer-encoding") != s.headers.end() && getHeaderValue(s.headers, "transfer-encoding") == "chunked")
			bodyHandlerFunc = &Request::unchunkBody;
		else
			throw(statusCodeException(501, "Not Implemented"));
	}
	else {
		string contentTypeValue = getHeaderValue(s.headers, "content-type");
		if (s.headers.find("content-length") != s.headers.end() && !contentTypeValue.empty() && isMultipartFormData(contentTypeValue))
		{
			boundary = "--" + contentTypeValue.substr(contentTypeValue.rfind('=')+1);
			length = w_stoi(getHeaderValue(s.headers, "content-length"));
			bodyHandlerFunc = &Request::contentlength;
			if (length > s.config.bodySize)
				throw(statusCodeException(413, "Request Entity Too Large"));
		}
		else
			throw(statusCodeException(501, "Not Implemented"));
	}
}
