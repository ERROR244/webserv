/*
    fork and run teh script and pipe and read it and ship it
    before check if the headers are well formed and make the start-line and send the response
*/

//https://www.ibm.com/docs/en/netcoolomnibus/8.1?topic=scripts-environment-variables-in-cgi-script

#include "requestParse.hpp"

void    prepearingCgiEnvVars() {
    unordered_map<string, string> env;
    env["AUTH_TYPE"] = "";
    env["CONTENT_LENGTH"] = -1;//content-length || the total length of the chunks;
    env["CONTENT_TYPE"] = "";//content-type;
    env["GATEWAY_INTERFACE"] = "CGI/1.1";//idk
    env["HTTP_ACCEPT"] = "";//idk
    env["HTTP_ACCEPT_CHARSET"] = "";//For example, utf-8;q=0.5.
    env["HTTP_ACCEPT_ENCODING"] = "";//For example, compress;q=0.5.
    env["HTTP_ACCEPT_LANGUAGE"] = "";//For example, en;q=0.5.
    env["HTTP_FORWARDED"] = "";//shows the address and port through of the proxy server.
    env["HTTP_HOST"] = "";//idk
    env["HTTP_PROXY_AUTHORIZATION"] = "";//idk
    env["HTTP_USER_AGENT"] = "Google chroome";
    env["PATH_INFO"] = "";//optionally URI contains extra infos after the scripts name;
    env["PATH_TRANSLATED"] = "";//idk
    env["QUERY_STRING"] = "";//?->
    env["REMOTE_ADDR"] = "";//idk
}