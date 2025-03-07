#!/usr/bin/python3

import os
import sys

# Ensure headers are sent before any content
print("Content-Type: text/html")
print()  # Required blank line after headers

# Get the content length from the environment
content_length = os.environ.get("HTTP_CONTENT_LENGTH", 0)

try:
    content_length = int(content_length)
except ValueError:
    content_length = 0

# Read the raw body from stdin based on content length
body = sys.stdin.read(content_length) if content_length > 0 else "(No body received)"

# Output response
print(f"""
<html>
<head><title>CGI POST Response</title></head>
<body>
    <h1>Received Raw Body</h1>
    <pre>{body}</pre>
</body>
</html>
""")
