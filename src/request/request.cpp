#include "httpSession.hpp"

httpSession::Request::Request(httpSession& session) : s(session), state(PROCESSING), length(0) {
	parseFunctions.push(&Request::parseStartLine);
	parseFunctions.push(&Request::parseFileds);
	parseFunctions.push(&Request::parseBody);

	bodyParseFunctions.push(&Request::boundary);
	bodyParseFunctions.push(&Request::fileHeaders);
	bodyParseFunctions.push(&Request::fileContent);
}

void	httpSession::Request::parseMessage(const int clientFd) {
	char*	buffer = new char[BUFFER_SIZE];
	ssize_t byteread;

	if ((byteread = recv(clientFd, buffer, BUFFER_SIZE, MSG_DONTWAIT)) <= 0) {
		state = CCLOSEDCON;
		return;
	}
	// cerr << "bytesread: " << byteread << endl;
	bstring clientRequest(buffer, byteread);
	delete[] buffer;
	clientRequest = remainingBuffer + clientRequest;
	remainingBuffer = NULL;
	// cerr << "---s-rawdata" << endl;
	// cerr << clientRequest << endl;
	// cerr << "---e-rawdata" << endl;
	while(!parseFunctions.empty()) {
		const auto& func = parseFunctions.front();
		if (!(this->*func)(clientRequest))	return;
		parseFunctions.pop();
	}
	state = DONE;
	cerr << "done parsing the request" << endl;
}

const t_state& httpSession::Request::status() const {
	return state;
}

// GET / HTTP1.1\r\n
//headersbody