#include "webServ.hpp"

int main(int ac, char **av) {
    if (ac != 2) {
        cout << "invalid number of argument" << endl;
        return 1;
    }
    (void)av;

    webServ wServ;

    try {
        wServ.createSockets();
        wServ.startEpoll();
        wServ.reqResp();
    }
    catch (const char *s) {
        cerr << s << endl;
        return -1;
    }


    return 0;
}