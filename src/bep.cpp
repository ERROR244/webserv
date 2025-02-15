#include "server.h"
void	f(stringstream& stream) {
	int length;
	string line;
	// if (!length) {
	// 	try {
	// 		length = stoi(s.headers["content-length"]);
	// 	}
	// 	catch(...) {
	// 		perror("unvalid number in content length"); throw(statusCodeException(500, "Internal Server Error"));
	// 	}
	// }

	while(getline(stream, line)) {
		
	}
}



// POST /upload HTTP/1.1
// Host: localhost:8080
// Content-Type: multipart/form-data; boundary=----WebKitFormBoundary12345

// ------WebKitFormBoundary12345
// Content-Disposition: form-data; name="file1"; filename="image1.jpg"
// Content-Type: image/jpeg

// (binary content for image1.jpg)

// ------WebKitFormBoundary12345
// Content-Disposition: form-data; name="file2"; filename="image2.jpg"
// Content-Type: image/jpeg

// (binary content for image2.jpg)

// ------WebKitFormBoundary12345--