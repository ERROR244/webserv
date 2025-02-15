#include "server.hpp"
#include "confiClass.hpp"

int main(int ac, char **av) {
    if (ac != 2) {
        cerr << "invalid number of argument" << endl;
    } else {
        map<string, configuration> config;
        try {
            ConfigFileParser confi(av[1]);
            config = confi.parseFile();
            confi.printprint();
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
        std::vector<int>    servrSocks;
        int                     epollFd;
    	createSockets(servrSocks, config);
    	epollFd = startEpoll(servrSocks);
        multiplexerSytm(servrSocks, epollFd, config);
	}
    return 0;
}