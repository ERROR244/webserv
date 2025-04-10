#include "wrappers.h"
#include "httpSession.hpp"

string	w_realpath(const char * file_name) {
	char absolutePath[1024];

	if (!realpath(file_name, absolutePath))
		throw(statusCodeException(404, "Not Found"));
	return string(absolutePath);
}

string toString(const int& nbr) {
    ostringstream oss;

    oss << nbr;
    return (oss.str());
}

int my_stoi(const std::string &str, size_t *pos = 0, int base = 10) {
	if (base != 10)
		throw std::invalid_argument("stoi only supports base 10");
	size_t	i = 0;
	size_t	startPos;
	int		sign = 1;
	long	result = 0;

	while (i < str.length() && isspace(str[i]))
		i++;

	if (i >= str.length())
		throw std::invalid_argument("stoi: no conversion could be performed");
	if (str[i] == '-' || str[i] == '+') {
		if (str[i] == '-')
			sign = -1;
		i++;
	}
	if (i >= str.length() || !isdigit(str[i]))
		throw std::invalid_argument("stoi: no valid digits found");

	startPos = i;
	while (i < str.length() && isdigit(str[i])) {
		// int digit = str[i] - '0';
		if (result > (INT_MAX - (str[i] - '0')) / 10)
			throw std::out_of_range("stoi: integer overflow");
		result = result * 10 + (str[i] - '0');
		i++;
	}
	if (pos)
		*pos = (i > startPos) ? i : 0;
	return static_cast<int>(sign * result);
}

int ft_stoi(const std::string &__str) {
	try {
		int res = my_stoi(__str);
		return (res);
	}
	catch (exception& e) {
		cout << e.what() << endl;
		throw std::runtime_error("invalid number: `" + __str + "`");
}

int w_stoi(const string& snum) {
    int num;
    try {
        num = my_stoi(snum);        //it will throw incase of invalid arg
    } catch (...) {
        perror("stoi failed");
        throw(statusCodeException(400, "Bad Request20"));
    }
    return num;
}

string ft_inet_ntoa(struct in_addr addr) {
    char buffer[16];
    uint32_t ip = ntohl(addr.s_addr);
    snprintf(buffer, sizeof(buffer), "%u.%u.%u.%u",
                (ip >> 24) & 0xFF,
                (ip >> 16) & 0xFF,
                (ip >> 8)  & 0xFF,
                ip & 0xFF);
    return string(buffer);
}

string getsockname(int clientFd) {
    struct sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    string res;

    if (getsockname(clientFd, (struct sockaddr*)&addr, &addrLen) == 0) {
        res = ft_inet_ntoa(addr.sin_addr) + ":" + toString(ntohs(addr.sin_port));
    } else {
        cerr << "getsockname failed" << endl;
        throw (statusCodeException(500, "Internal Server Error"));
    }
    return res;
}

int ft_socket(int __domain, int __type, int __protocol) {
    int fd = socket(__domain, __type, __protocol);
    if (fd == -1) {
        throw std::runtime_error("Socket creation failed");
    }
    return fd;
}

int ft_setsockopt(int __fd, int __level, int __optname) {
    int opt = 1;
    if (setsockopt(__fd, __level, __optname, &opt, sizeof(opt)) < 0) {
        ft_close(__fd, "ft_setsockopt");
        throw std::runtime_error("setsockopt failed");
    }
    return 0;
}

// int ft_fcntl(int __fd, int __cmd1, int __cmd2) {
//     if (fcntl(__fd, __cmd1, __cmd2) < 0) {
//         ft_close(__fd, "ft_fcntl");
//         throw "setsockopt failed";
//     }
//     return 0;
// }

int ft_bind(int __fd, const sockaddr *__addr, socklen_t __len) {
    if (bind(__fd, __addr, __len) < 0) {
        ft_close(__fd, "ft_bind");
        throw std::runtime_error("Bind failed: " + string(strerror(errno)));
    }
    return 0;
}

int ft_listen(int __fd, int __n) {
    if (listen(__fd, __n) < 0) {
        ft_close(__fd, "ft_listen");
        throw std::runtime_error("listen failed");
    }
    return 0;
}

int ft_close(int& __fd, string who) {
    if (__fd < 0 || close(__fd) < 0) {
        cerr << "close failed: " << who << endl;
    }
    __fd = -1;
    return 0;
}

int ft_epoll_create1(int __flags) {
    int epollFd = epoll_create1(__flags);
    if (epollFd == -1) {
        throw std::runtime_error("epoll_create1 failed");
    }
    return epollFd;
}

int ft_epoll_ctl(int __epfd, int __op, int __fd, epoll_event *event) {
    if (epoll_ctl(__epfd, __op, __fd, event) < 0) {
        ft_close(__epfd, "ft_epoll_ctl");
        throw std::runtime_error("Epoll ctl failed");
    }
    return 0;
}

int ft_epoll_wait(int __epfd, epoll_event *__events, int __maxevents, int __timeout, map<int, httpSession> sessions, int epollFd) {
    int nfds;

    if ((nfds = epoll_wait(__epfd, __events, __maxevents, __timeout)) == -1) {
        ft_perror("epoll_wait failed");
        if (errno == EBADF || errno == ENOMEM) {
            //close all client's connections
            for (map<int, httpSession>::iterator it = sessions.begin(); it != sessions.end(); ++it) {
                close(it->first);
            }
            close(epollFd);
            return -1;
        }
        return 0;
    }
    return nfds;
}

int ft_epoll__ctl(int __epfd, int __fd, epoll_event* ev) {
    if (epoll_ctl(__epfd, EPOLL_CTL_MOD, __fd, ev) == -1) {
        cerr << "epoll__ctl failed" << endl;
        close(__fd);
        return -1;
    }
    return 0;
}

void ft_perror(const std::string &msg) {
    if (!msg.empty())
        std::cerr << msg << ": ";
    std::cerr << std::strerror(errno) << std::endl;
}
