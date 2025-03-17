#include "server.h"
#include "confiClass.hpp"
#include <termios.h>

void disableEchoCtrl() {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag &= ~ECHOCTL; // Disable printing of control characters (like ^C)
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

void signalHandler( int signum ) {
   cout << "Interrupt signal (" << signum << ") received.\n";
   exit(signum);
}

int main(int ac, char **av) {
    map<string, configuration> config;
    vector<int> serverFds;
    int epollFd;
    
    if (ac != 2) {
        cerr << "invalid number of argument" << endl;
    } else {
        //config file
        disableEchoCtrl();
        signal(SIGINT, signalHandler);
        //multiplexer
        try {
            ConfigFileParser confi(av[1]);
            config = confi.parseFile();
            confi.printprint();
            epollFd = createSockets(config, serverFds);     
            while (1) {                                                 //this loop is here if epoll fd somehow got closed and epoll wait fails and i have to create and instance of epoll fd;
                multiplexerSytm(serverFds, epollFd, config);
                epollFd = startEpoll(serverFds);
            }
        }
        catch (const exception& msg) {
            cerr << msg.what() << endl;
            cerr << "msg.what()" << endl;
            return -1;
        }
	}
    return 0;
}