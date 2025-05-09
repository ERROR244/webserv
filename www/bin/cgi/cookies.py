#!/usr/bin/env python3

import os

cookie_header = os.getenv("HTTP_COOKIE", "")
cookies = {}
if cookie_header:
    for cookie in cookie_header.split(";"):
        if "=" in cookie:
            name, value = cookie.strip().split("=", 1)
            cookies[name] = value

# Generate the response body
response_lines = []
response_lines.append("<html>")
response_lines.append("<head><title>Cookie Test</title></head>")
response_lines.append("<body>")
response_lines.append("<h1>Cookie Test</h1>")

# Display incoming cookies
if cookies:
    response_lines.append("<h2>Incoming Cookies:</h2>")
    response_lines.append("<ul>")
    for name, value in cookies.items():
        response_lines.append(f"<li><strong>{name}:</strong> {value}</li>")
    response_lines.append("</ul>")
else:
    response_lines.append("<p>No incoming cookies.</p>")

# Confirm outgoing cookies
response_lines.append("<h2>Outgoing Cookies:</h2>")
response_lines.append("<p>Cookies have been set:</p>")
response_lines.append("<ul>")
if "username" not in cookies:
    response_lines.append("<li><strong>username:</strong> JohnDoe</li>")
if "theme" not in cookies:
    response_lines.append("<li><strong>theme:</strong> dark</li>")
response_lines.append("</ul>")

response_lines.append("</body>")
response_lines.append("</html>")

response_body = "\n".join(response_lines)
response_bytes = response_body.encode("utf-8")
content_length = len(response_bytes)

# Print CGI headers
if "username" not in cookies:
    print("Set-Cookie: username=JohnDoe; Path=/; HttpOnly")
if "theme" not in cookies:
    print("Set-Cookie: theme=dark; Path=/; Max-Age=3600")
print("Content-Type: text/html")
print(f"Content-Length: {content_length}")
print()  # End of headers

# Print response body
print(response_body)
