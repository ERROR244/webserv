#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

int main(int ac, char **av) {
    struct sockaddr_in addr;

    int clientFd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientFd == -1) {
        perror("Socket creation failed");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) {
        perror("Invalid address");
        return 1;
    }

    if (connect(clientFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Connect failed");
        close(clientFd);
        return 1;
    }

	string ss = "";
	while(true) {
		string line;
		getline(cin, line);
		if (line != "END") {
			ss += line + "\r\n";
		}
		else {
			send(clientFd, ss.c_str(), ss.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
			ss = "";
		}
	}

    close(clientFd);
    return 0;
}


