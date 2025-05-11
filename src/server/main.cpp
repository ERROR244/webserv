#include "server.h"
#include "confiClass.hpp"
#include <termios.h>

bool shouldStop(int i) {
	static volatile sig_atomic_t stopFlag = 0;

	if (i == 0)
		return (stopFlag == 0);
	stopFlag = i;
	return false;
}

void disableEchoCtrl() {
	struct termios tty;
	tcgetattr(STDIN_FILENO, &tty);
	tty.c_lflag &= ~ECHOCTL; // Disable printing of control characters (like ^C)
	tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

void signalHandler( int signum ) {
	(void)signum;
	shouldStop(1);
}

int main(int ac, char **av, char **envp) {
	map<string, configuration> config;
	vector<int> serverFds;
	int epollFd;
	
	if (ac != 2) {
		cerr << "invalid number of argument" << endl;
		return 0;
	} else {
		try {
			//settin home envs
			homeEnvVariables(envp);
			//handling program signals
			disableEchoCtrl();
			signal(SIGINT, signalHandler);
			//setup server
			ConfigFileParser confi(av[1]);
			config = confi.parseFile();
			epollFd = createSockets(config, serverFds);     
			//multiplexer
			while (true) {
				multiplexerSytm(epollFd, config);
				if (shouldStop(0) == false)
					break;
				epollFd = startEpoll(serverFds);
			}
		}
		catch (const exception& msg) {
			cerr << msg.what() << endl;
			return -1;
		}
	}
	cout << "Interrupt signal 'SIGINT' received.\n" << "the server has been stoped" << endl;
	return 0;
}