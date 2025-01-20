#include "webServ.hpp"

// webServ::webServ(int fd) : fd(fd) { }

// webServ::webServ(const webServ& other) {
//     *this = other;
// }
// webServ& webServ::operator=(const webServ& other) {
//     if (this != &other) {
//         this->fd = other.fd;
//     }
//     return (*this);
// }

// webServ::~webServ() { }

vector<int> webServ::getPorts() {
    vector<int> ports;
    ports.push_back(8080);
    ports.push_back(9090);
    return (ports);
}

void webServ::startSocket(const int& port) {
    int fd = ft_socket(AF_INET, SOCK_STREAM, 0); // create a socket

    ft_setsockopt(fd, SOL_SOCKET, SO_REUSEADDR); // configure socket options, see man setsockopt

    ft_fcntl(fd, F_SETFL, O_NONBLOCK);

    struct sockaddr_in address = {};
    address.sin_family = AF_INET;          // AF_INET = ipv4
    address.sin_addr.s_addr = INADDR_ANY;  // INADDR_ANY = listen on all network interfaces
    address.sin_port = htons(port);        // convert port to network byte order
    
    ft_bind(fd, (struct sockaddr*)&address, sizeof(address)); // bind socket to its corresponding port.
    ft_listen(fd, 3);  // the server waits for someone to connect (like a browser).
    serverFd.push_back(fd); // add that socket the serverFd
}

// initialize 1 socket for each port
void webServ::createSockets() {
    extensions = getSupportedeExtensions();
    vector<int> ports = getPorts();
    for (int port = 0; port < (int)ports.size(); ++port) {
        startSocket(ports[port]);
    }
}

// startEpoll and add server sockets to the epoll instance
void webServ::startEpoll() {
    epollFd = ft_epoll_create1(0); // Create an epoll instance

    struct epoll_event event = {};
    for (int fd = 0; fd < (int)serverFd.size(); ++fd) {
        event.events = EPOLLIN;       // monitor for incoming connections
        event.data.fd = serverFd[fd]; // associate the file descriptor
        ft_epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd[fd], &event);
    }
}

void webServ::reqResp() {
    int nfds;

    cout << "Server listening on port XX..." << endl;
    while (true) {
        // wait for events on the monitored sockets
        nfds = ft_epoll_wait(epollFd, events, MAX_EVENTS, -1, serverFd, epollFd);

        for (int i = 0; i < nfds; i++) {
            if (find(serverFd.begin(), serverFd.end(), events[i].data.fd) != serverFd.end()) {
                handelNewConnection(events[i].data.fd);
            }
            else {

                handelClient(i); // req
                handelClient(i); // header or the full respones if the file size < 10000
                handelClient(i); // chunk
                handelClient(i); // chunk end 
            }
        }
    }
}

