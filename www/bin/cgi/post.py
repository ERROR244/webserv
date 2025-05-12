#!/usr/bin/python3

import os
import sys

# Get the content length from the environment
content_length = os.environ.get("CONTENT_LENGTH", 0)

try:
    content_length = int(content_length)
except ValueError:
    content_length = 0

# Read the raw body from stdin
body = sys.stdin.read(content_length) if content_length > 0 else "(No body received)"

# Create the response body
response_b
ody = f"""<html>
<head><title>CGI POST Response</title></head>
<body>
    <h1>Received Raw Body</h1>
    <pre>{body}</pre>
</body>
</html>
"""

# Encode to bytes and calculate Content-Length
response_bytes = response_body.encode("utf-8")
content_length = len(response_bytes)

# Output headers
print("Content-Type: text/html")
print(f"Content-Length: {content_length}")
print()  # End of headers

# Output body
print(response_body)
