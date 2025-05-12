#!/usr/bin/env python3

import cgi

# Generate HTML content
html_content = """\
<html>
<head><title>CGI Test</title></head>
<body>
<h1>Hello, CGI!</h1>
"""

# Get form data
form = cgi.FieldStorage()
name = form.getvalue("name", "Guest")

html_content += f"<p>Welcome, {name}!</p>"
html_content += "</body></html>"

# Compute content length
content_length = len(html_content.encode())  # Encode to bytes before measuring

# Print headers
print("Content-Type: text/html")
# print(f"Content-Length: {content_length}")  # Set Content-Length
print()  # Empty line to separate headers from body

# Print content
print(html_content)
