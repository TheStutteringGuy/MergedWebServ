#!/usr/bin/env python3
import os
import sys
import time
import urllib.parse

# --- 1. Get Incoming CGI Environment Data ---
method = os.environ.get("REQUEST_METHOD", "N/A")
query_string = os.environ.get("QUERY_STRING", "N/A")
# The incoming cookie data (what the browser sent to your server)
incoming_http_cookie = os.environ.get("HTTP_COOKIE", "None found") 
content_length = os.environ.get("CONTENT_LENGTH")
post_body = ""

# Read POST body data if available
if content_length and content_length.isdigit() and int(content_length) > 0:
    try:
        post_body = sys.stdin.read(int(content_length))
    except Exception as e:
        post_body = f"ERROR READING BODY: {e}"

# --- 2. Determine Cookie Attributes ---
# Use the current time for the cookie value
new_visit_timestamp = int(time.time())

# **CRITICAL IMPROVEMENT:** Set Path=/ to ensure it's sent for all URLs.
# Max-Age is set to 1 hour (3600 seconds) for easy testing.
# If you are testing on HTTPS, add '; Secure'
cookie_attributes = f"Max-Age=3600; Path=/" 
# Example for HTTPS: cookie_attributes = f"Max-Age=3600; Path=/; Secure" 

# --- 3. Write HTTP Response Headers ---
sys.stdout.write("Status: 200 OK\r\n")
sys.stdout.write("Content-Type: text/plain\r\n")

# **IMPROVED Set-Cookie Header:** This should fix most rejection issues.
sys.stdout.write(f"Set-Cookie: LastVisit={new_visit_timestamp}; {cookie_attributes}\r\n")
sys.stdout.write("\r\n") # Mandatory empty line separates headers from body

# --- 4. Write Content Body (Debug Output) ---
sys.stdout.write(f"--- COOKIE TEST RESULTS ---\n")
sys.stdout.write(f"1. **NEW Set-Cookie Value (Sent to Browser):** LastVisit={new_visit_timestamp}\n")
sys.stdout.write(f"2. **INCOMING Cookie (Received from Browser):** {incoming_http_cookie}\n")
sys.stdout.write(f"3. Request Method: {method}\n")
sys.stdout.write(f"4. Query String: {query_string}\n")
sys.stdout.write(f"5. Body Data Read: {post_body[:50]}...\n")
sys.stdout.write(f"--- END TEST ---\n")