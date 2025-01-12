#include "request_parse.cpp"

int main() {
	string req = "GET /test?123=123 HTTP/1.1\r\nHost: test\r\nTransfer-encoding:\r\n\r\nhello world";
	try {
		Request test(req);
	} catch (const char* err) {
		cerr << err << endl;
	}
	return (0);
}