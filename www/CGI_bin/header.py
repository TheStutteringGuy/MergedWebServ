#!/usr/bin/env python3

import os
import sys

print("Content-Type: text/html")
print()

print("<html>")
print("<head><title>Python CGI Test</title></head>")
print("<body style='font-family: monospace;'>")

print("<h1>🐍 Python CGI Success!</h1>")
print("<hr>")

print("<h2>Basic Information:</h2>")
print(f"<p><strong>Python Version:</strong> {sys.version.split()[0]}</p>")
print(f"<p><strong>Script Name:</strong> {os.environ.get('SCRIPT_NAME', 'N/A')}</p>")

print("<h2>Environment Variables:</h2>")
print("<ul>")
for key, value in sorted(os.environ.items()):
    print(f"<li><strong>{key}:</strong> {value}</li>")
print("</ul>")

print("</body>")
print("</html>")