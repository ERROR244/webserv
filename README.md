[![42 Project](https://img.shields.io/badge/42-Project-blue.svg)](https://42.fr/)
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![License](https://img.shields.io/badge/license-MIT-lightgrey.svg)]()
# webserv

A lightweight HTTP server written in **C++98**, replicating key features of modern web servers like NGINX. It includes configuration parsing, CGI execution, autoindexing, and supports multiple HTTP methods.

> Developed for the 42 Network curriculum.

---

## ⚙️ Features

- HTTP 1.1 compliant server
- Supports `GET`, `POST`, `DELETE`
- Parses a custom `.conf` configuration file
- Multi-location, multi-server support
- CGI execution (`.py`, `.php`, `.sh`)
- Autoindexing (directory listing)
- Custom error pages (400–500)
- File upload handling
- Path aliasing and redirection
- Request method filtering
- Epoll-based I/O multiplexing

---

## 🗂️ Project Structure

```bash
.
├── confi.conf                  # Configuration file
├── includes/                  # Header files
├── src/                       # Source code
│   ├── bstring/               # Custom string class
│   ├── cgi/                   # CGI handling logic
│   ├── confi/                 # Config file parsing & classes
│   ├── exceptions/            # Custom exception types
│   ├── parsers/               # HTTP request parsing
│   ├── response/              # HTTP response logic
│   ├── server/                # Server setup, event loop, etc.
│   └── wrappers.cpp           # Low-level sys call wrappers
├── www/                       # Web root
│   ├── static/                # Static content (HTML, CSS, assets)
│   ├── uploads/               # Upload destination
│   └── bin/cgi/               # CGI scripts
└── Makefile                   # Build file
```


---

## 🔧 Getting Started

### ✅ Prerequisites

- C++98 compatible compiler (e.g. `g++`)
- `make`
- UNIX-based system (Linux/macOS)
- `python3`, `php-cgi`, `bash` (for CGI execution)

### 🛠️ Build

```bash
make
```

### 🛠️ run

```bash
./webserv confi.conf
```

### 🧾 Configuration Example

```bash
server {
	listen:     localhost:8080
	limit_req:  11231313

	errors {
		404 ./www/static/errors/404.html
		500 ./www/static/errors/500.html
	}

	locations {
		location / {
			index: index.html
			alias: /www/static/html/
			autoindex: on
			uploads: www/uploads
			limit_except: GET, POST, DELETE
			cgi {
				add-handler: .py /usr/bin/python3
				add-handler: .php /usr/bin/php-cgi
			}
		}
	}
    # Additional location blocks...
}
```

### 🧪 Testing
You can test the server using:

- Browser: Go to http://localhost:8080/
- URL/Postman: For method-specific requests like DELETE, POST, etc.
- CGI Scripts: Place .py, .php, or .sh scripts under /www/bin/cgi/

### 📁 Example Content
- www/static/html/index.html: Default homepage
- www/static/assets/: Images, videos, and other assets
- www/bin/cgi/: Sample CGI scripts (Python, PHP, ...)
- www/uploads/: Endpoint to test file uploads

### 🧠 Learnings

- Mastered socket programming and epoll-based multiplexing
- Built a full HTTP parser and response system from scratch
- Parsed custom .conf files without third-party parsers
- Hands-on with real-world features like CGI and uploads

### 👨‍💻 Authors

- [ERROR244](https://github.com/ERROR244)
- [nabilaadou](https://github.com/nabilaadou)

### 📜 License
MIT — Free to use, modify, and share.
