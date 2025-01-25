/*
	fork and run teh script and pipe and read it and ship it
	before check if the headers are well formed and make the start-line and send the response
*/

//https://www.ibm.com/docs/en/netcoolomnibus/8.1?topic=scripts-environment-variables-in-cgi-script

#include "Cgi.hpp"

void	Cgi::prepearingCgiEnvVars(Request req) {
	unordered_map<string, string> env;

	env["GATEWAY_INTERFACE"] = "CGI/1.1";//idk
	env["SERVER_PROTOCOL"] = "http/1.1";
	env["SERVER_NAME"] = "localhost";
	env["REMOTE_METHODE"] = req.getMethod();
	env["CONTENT_LENGTH"] = req.getHeader("content-length");
	env["CONTENT_TYPE"] = req.getHeader("content-type");
	env["HTTP_ACCEPT"] = req.getHeader("accept");
	env["HTTP_ACCEPT_CHARSET"] = req.getHeader("accept-charset");
	env["HTTP_ACCEPT_ENCODING"] = req.getHeader("accept-encoding");
	env["HTTP_ACCEPT_LANGUAGE"] = req.getHeader("accept-language");
	env["HTTP_USER_AGENT"] = req.getHeader("user-agent");
	env["HTTP_HOST"] = "";//idk
	env["PATH_INFO"] = "";//extract it manually from the target uri
	env["PATH_TRANSLATED"] = "";//idk
	env["QUERY_STRING"] = "";//?->
	env["REMOTE_ADDR"] = "";//idk
	env["REMOTE_HOST"] = "";
	env["REMOTE_USER"] = "";
	env["SCRIPT_NAME"] = "";
	env["SERVER_PORT"] = "";
	env["WEBTOP_USER"] = "";
}