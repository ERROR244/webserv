#include "webServ.hpp"

int main(int ac, char **av) {
    if (ac != 2) {
        cout << "invalid number of argument" << endl;
        return 1;
    }

    try {
        webServ wServ(av[1]);

        // wServ.createSockets();
        // wServ.startEpoll();
        // wServ.reqResp();
    }
    catch (const char *s) {
        cerr << s << endl;
        return -1;
    }
    catch (string s) {
        cerr << s << endl;
        return -1;
    }
    catch (exception& e) {
        cerr << e.what() << endl;
        return -1;
    }


    return 0;
}