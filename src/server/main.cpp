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
            // confi.printprint();
            //server setuping
            epollFd = createSockets(config, serverFds);
        }
        catch (const char *s) {
            cerr << s << endl;
            return -1;
        }
        catch (string s) {
            cerr << s << endl;
            return -1;
        }
        catch (...) {
            cerr << "ERROR" << endl;
            return -1;
        }
        //multiplexer
        multiplexerSytm(serverFds, epollFd, config);
	}
    return 0;
}