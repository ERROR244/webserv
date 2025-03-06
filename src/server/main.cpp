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

void getConfi(char *fileName, map<string, configuration>& config, int& epollFd, vector<int> &serverFds) {
    try {
        ConfigFileParser confi(fileName);
        config = confi.parseFile();
        // confi.printprint();
        epollFd = createSockets(config, serverFds);
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        exit(-1);
    }
    catch (...) {
        cerr << "ERROR" << endl;
        exit(-1);
    }
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
        getConfi(av[1], config, epollFd, serverFds);       
        //multiplexer
        try {
            multiplexerSytm(serverFds, epollFd, config);
        }
        catch (...) {
            cerr << "server error" << endl;
        }
	}
    return 0;
}