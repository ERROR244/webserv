#include "server.h"
#include "confiClass.hpp"

int main(int ac, char **av) {
    if (ac != 2) {
        cerr << "invalid number of argument" << endl;
    } else {
        //config file
        vector<int> serverFds;
        int epollFd;
        map<string, configuration> config;
        try {
            ConfigFileParser confi(av[1]);
            config = confi.parseFile();
            confi.printprint();
            //server setuping
            epollFd = createSockets(config, serverFds);
        }
        catch (const std::runtime_error& error) {
            cerr << error.what() << endl;
            return -1;
        }
        catch (...) {
            cerr << "ERROR" << endl;
            return -1;
        }
        //multiplexer
        try {
            multiplexerSytm(serverFds, epollFd, config);
        }
        catch (const statusCodeException& exception) {
            struct epoll_event	ev;
            cerr << "2D code--> " << exception.code() << endl;
            cerr << "2D reason--> " << exception.meaning() << endl;
        }
	}
    return 0;
}