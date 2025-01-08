#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

int main() {
    /*
        building a tcp socket
    */
    int clientFd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientFd < 0) {
        perror("socket failed");
        exit(0);
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET; // AF_INET defining the protocol to be used(IPV4)
    serverAddress.sin_port = htons(8080); // PORT
    serverAddress.sin_addr.s_addr = INADDR_ANY; // deciding which IPs can connect to the same socket

	connect(clientFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

	const char* msg = "hello, server !!";
	send(clientFd, msg, strlen(msg), 0);


    close(clientFd);
}