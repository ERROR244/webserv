import socket

def send_http_request(host, port, request_text):
    # Create a TCP socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((host, port))
        sock.sendall(request_text.encode('utf-8'))
        
        response = b""
        while True:
            part = sock.recv(4096)
            if not part:
                break
            response += part

    return response.decode('utf-8')

if __name__ == "__main__":
    # Target server
    host = "127.0.0.1"
    port = 8080

    # Craft a simple HTTP GET request
    request = (
        "POST /bin/cgi/script.py HTTP/1.1\r\n"
        f"Host: {host}\r\n"
        "Connection: close\r\n"
        "transfer-encoding: chunked\r\n"
        "\r\n"
        "5\r\n"
        "hello\r\n"
        "0\r\n\r\n"
    )
    # request = (
    #     "GET / HTTP/1.1\r\n"
    #     "\r\n"
    # )

    # Send request and print the response
    response = send_http_request(host, port, request)
    print("=== Server Response ===")
    print(response)