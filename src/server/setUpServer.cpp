#include "server.h"

int	addServrSocksToEpollPool(map<int, t_sockaddr>& servrSocks) {
	int					epollFd;
	struct epoll_event	ev;

	if ((epollFd = epoll_create1(0)) == -1) {
		perror("epoll_create1 failed(setUpserver.cpp)"); exit(-1);
	}
	for (t_sockaddr_it it = servrSocks.begin(); it != servrSocks.end(); ++it) {
		ev.events = EPOLLIN;
		ev.data.fd = it->first;
		if (epoll_ctl(epollFd, EPOLL_CTL_ADD, it->first, &ev) == -1) {
			perror("epoll_ctl failed"); exit(-1);
		}
	}
	return epollFd;
}

int	startServer(map<int, t_sockaddr>& servrSocks) {
	vector<int>	ports;

	ports.push_back(8080);
	ports.push_back(4040);
	for (int i = 0; i < ports.size(); ++i) {
		t_sockaddr	sockInfo;
		int 		sockFd;
		int			opt = 1;
		socklen_t	addrLen = sizeof(t_sockaddr);

		if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    	    perror("socket failed(setUpserver.cpp)"); exit(-1);
    	}
		if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
    	    perror("setsockopt failed(setUpserver.cpp)"); exit(-1);
    	}
		sockInfo.sin_family = AF_INET;
    	sockInfo.sin_addr.s_addr = INADDR_ANY;
    	sockInfo.sin_port = htons(ports[i]);
		if (bind(sockFd, (struct sockaddr*)&sockInfo, addrLen) < 0) {
    	    perror("bind failed(setUpserver.cpp): "); exit(-1);
    	}
		cout << "listening on port XXXX.." << endl;
		if (listen(sockFd, 3) < 0) {
    	    perror("listen failed(setUpserver.cpp): "); exit(-1);
    	}
		servrSocks[sockFd] = sockInfo;
	}
	int epollFd = addServrSocksToEpollPool(servrSocks);
	return epollFd;
}