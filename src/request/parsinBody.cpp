#include "httpSession.hpp"

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

static int	openFile(const string& value, const string& path) {
	int				fd;
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
	if ((fd = open((path + "/" + keyvalue[1]).c_str(), O_CREAT | O_WRONLY, 0644)) < 0) {
		perror("open failed");
		throw(statusCodeException(500, "Internal Server Error"));
	}
	return fd;
}

void	httpSession::Request::parseBody(const bstring& buffer, size_t pos) {
	size_t				len = 0;
	size_t				contentStartinPos = pos;
	size_t				size = buffer.size();
	char				ch;
	map<string, string>	contentHeaders;

	while (pos < size) {
		ch = buffer[pos];
		switch (s.sstat)
		{
		case e_sstat::bodyFormat: {
				if (s.headers.find("content-type") != s.headers.end() && isMultipartFormData(s.headers["content-type"])) {
					boundary = "--" + s.headers["content-type"].substr(s.headers["content-type"].rfind('=')+1);
					if (s.headers.find("content-length") != s.headers.end()) {
						s.sstat = e_sstat::contentLengthBased;
						try {
							length = stoi(s.headers["content-length"]);//it will throw incase of invalid arg
						} catch (...) {
							perror("stoi failed");
							throw(statusCodeException(500, "Internal Server Error"));//not really an internall error
						}
					}
					else if (s.headers.find("transfer-encoding") != s.headers.end() && s.headers["transfer-encoding"] == "chunked")
						s.sstat = e_sstat::transferEncodingChunkedBased;
				} else
					throw(statusCodeException(501, "Not Implemented"));
		}
		case e_sstat::contentLengthBased: {
			if (ch == '\n') {
				bool	crInLine = false;

				if (pos && buffer[pos-1] == '\r')
					crInLine = true;
				if (!buffer.ncmp(boundary.c_str(), len-crInLine, pos-len)) {
					size_t	newPos;
					size_t	boundaryStartinIndex = pos-len;

					if (fd != -1) {
						++len;//including pre boundary nl
						if (pos && buffer[pos-len-1] == '\r')
							++len;//includin the CR if it exist
						write(fd, &(buffer[contentStartinPos]), pos-contentStartinPos-len);
					}
					s.sstat = e_sstat::emptyline;
					if ((newPos = parseFields(buffer, pos+1, contentHeaders)) < 0) {
						remainingBody = buffer.substr(boundaryStartinIndex);
						fd = -1;
						return;
					}
					pos = newPos;
					s.sstat = e_sstat::contentLengthBased;
					contentStartinPos = pos;
					fd = openFile(contentHeaders["content-disposition"], s.path);
				}
				else if (!buffer.ncmp((boundary+"--").c_str(), len-crInLine, pos-len)) {
					++len;//including pre boundary nl
					if (pos && buffer[pos-len-1] == '\r')
						++len;
					write(fd, &(buffer[contentStartinPos]), pos-contentStartinPos-len);
					s.sstat = e_sstat::sHeader;
					return;
				}
				len = 0;
				++pos;
				continue;
			}
			++len;
		}
		// case e_sstat::transferEncodingChunkedBased:
		// default:
		// 	break;
		// }
		++pos;
		}
	}
	/* 
		check the lenght of len if its higher the boundary then just write everythinh and no need to save anything but if its less
		there's a possibility of it being the unfinished line of the boundary
	*/
	remainingBody = NULL;
	if (len <= boundary.size()+2) {//check if the line is smaller than the boundary
		++len;
		if (pos && pos-len-1 == '\r')
			++len;
		remainingBody = buffer.substr(pos-len);
		write(fd, &(buffer[contentStartinPos]), pos-contentStartinPos-len);
	} else {
		write(fd, &(buffer[contentStartinPos]), pos-contentStartinPos);
	}
}  