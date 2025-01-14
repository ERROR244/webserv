#include "requestParse.cpp"

int main() {
	string req = "POST http://localhost/test?123=123 HTTP/1.1\r\nhost: localhost\r\ntransfer-encoding: chunked\r\n\r\n7\r\nMozilla\r\n17\r\nDeveloper Network\r\n0\r\n\r\n";
	try {
		stringstream stream(req);
		Request test(stream);
		stream.clear();
	} catch (const char* err) {
		cerr << err << endl;
	}
	return (0);
}