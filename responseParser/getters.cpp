#include "requestParse.hpp"

const string&	Request::getMethod()	const {
	return (method);
}

const string&	Request::getTarget()	const {
	return (target);
}

const string&	Request::getHttpProtocole()	const {
	return (httpVersion);
}

const string	Request::getHeader(const string& header) {
	if (headers.find(header) != headers.end())
		return (headers[header]);
	return "";
}

const string&	Request::getPath()	const {
	return (targetPath);
}

const string&	Request::getQuery()	const {
	return (targetQuery);
}

const string&	Request::getScriptName()	const {
	return (scriptName);
}

const bool&	Request::getRequestStatus() const {
	return readAllRequest;
}