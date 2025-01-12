#include "requestParse.cpp"

int main() {
	string req = "GET http://localhost/test?123=123 HTTP/1.1\r\nhost: localhost\r\nTransfer-encoding:\r\n\r\nhello world";
	try {
		Request test(req);
	} catch (const char* err) {
		cerr << err << endl;
	}
	return (0);
}