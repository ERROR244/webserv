#include "requestParse.hpp"

Request::Request() {
	parseFunctions.push(&Request::parseBody);
	parseFunctions.push(&Request::parseFileds);
	parseFunctions.push(&Request::parseStartLine);

	parseFunctionsStarterLine.push(&Request::isProtocole);
	parseFunctionsStarterLine.push(&Request::isTarget);
	parseFunctionsStarterLine.push(&Request::isMethod);
}


void	Request::parseMessage(const int clientFd) {
	char	buffer[BUFFER_SIZE+1] = {0};
	int		byteRead;
	if ((byteRead = recv(clientFd, buffer, BUFFER_SIZE, MSG_DONTWAIT)) < 0) {
		perror("recv syscall failed");
		exit(-1);
	}

	remainingBuffer += buffer;
	replace(remainingBuffer.begin(), remainingBuffer.end(), '\r', ' ');
	stringstream	stream(remainingBuffer);
	while(!parseFunctions.empty()) {
		const auto& func = parseFunctions.top();
		if (!(this->*func)(stream))	return;
		parseFunctions.pop();
	}
	readAllRequest = true;
}