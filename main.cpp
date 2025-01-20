#include "requestParse.cpp"


# include <fcntl.h>
# include <stddef.h>
# include <stdlib.h>
# include <unistd.h>
# include <limits.h>

string getnline(int fd) {
	char c[1] = {0};
	string line = "";
	while (1) {
		int f = read(fd, c, 1);
		if (f <= 0 || *c == 0)
			break;
		else if (*c == '\n') {
			line += '\n';
			break;
		}
		line += *c;
	}
	return (line);
}

int main() {
	try {
		Request req;
		while (1) {
			string line = getnline(0);
			cerr << endl;
			req.parseMessage(line.c_str());
		}
	} catch (const char* err) {
		cerr << err << endl;
	}
	return (0);
}