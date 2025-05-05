#include "wrappers.h"
#include "server.h"
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

off64_t my_stoi(const string &str, size_t *pos, int base) {
	if (base != 10)
		throw invalid_argument("stoi only supports base 10");
	size_t	i = 0;
	size_t	startPos;
	int		sign = 1;
	off64_t	result = 0;

	while (i < str.length() && isspace(str[i]))
		i++;

	if (i >= str.length())
		throw invalid_argument("stoi: no conversion could be performed");
	if (str[i] == '-' || str[i] == '+') {
		if (str[i] == '-')
			sign = -1;
		i++;
	}
	if (i >= str.length() || !isdigit(str[i]))
		throw invalid_argument("stoi: no valid digits found");

	startPos = i;
	while (i < str.length() && isdigit(str[i])) {
		if (result > (INT_MAX - (str[i] - '0')) / 10)
			throw out_of_range("stoi: integer overflow");
		result = result * 10 + (str[i] - '0');
		i++;
	}
	if (pos)
		*pos = (i > startPos) ? i : 0;
	return static_cast<int>(sign * result);
}

off64_t ft_stoi(const string &__str) {
	try {
		off64_t res = my_stoi(__str);
		return (res);
	}
	catch (exception& e) {
		throw runtime_error("invalid number: `" + __str + "`");
	}
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

string  ft_getsockname(int clientFd) {
	t_sockaddr  addr;
	socklen_t	addrLen = sizeof(addr);
	string		socketName;

	if (getsockname(clientFd, (struct sockaddr*)&addr, &addrLen) == 0) {
		socketName = ft_inet_ntoa(addr.sin_addr) + ":" + toString(ntohs(addr.sin_port));
	} else {
		cerr << "getsockname failed" << endl;
		throw (statusCodeException(500, "Internal Server Error"));
	}
	return socketName;
}

int ft_socket(int __domain, int __type, int __protocol) {
	int fd = socket(__domain, __type, __protocol);
	if (fd == -1) {
		throw runtime_error("Socket creation failed");
	}
	return fd;
}

int ft_setsockopt(int __fd, int __level, int __optname) {
	int opt = 1;
	if (setsockopt(__fd, __level, __optname, &opt, sizeof(opt)) < 0) {
		ft_close(__fd, "ft_setsockopt");
		throw runtime_error("setsockopt failed");
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
		throw runtime_error("Bind failed: " + string(strerror(errno)));
	}
	return 0;
}

int ft_listen(int __fd, int __n) {
	if (listen(__fd, __n) < 0) {
		ft_close(__fd, "ft_listen");
		throw runtime_error("listen failed");
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
		throw runtime_error("epoll_create1 failed");
	}
	return epollFd;
}