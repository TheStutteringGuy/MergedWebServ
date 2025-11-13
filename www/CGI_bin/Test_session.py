#!/usr/bin/env python3
import os
import sys
import time
import uuid
import http.cookies

# --- Session Configuration ---
SESSION_COOKIE_NAME = "SESSION_ID"
VISIT_COUNT_COOKIE_NAME = "VisitCount"
MAX_AGE_SECONDS = 600  # Session lasts 10 minutes (for testing)

# --- Initialize ---
method = os.environ.get("REQUEST_METHOD", "GET")
incoming_cookie_header = os.environ.get("HTTP_COOKIE", "")
session_id = None
visit_count = 0

# 1. Parse incoming cookies
try:
    C = http.cookies.SimpleCookie(incoming_cookie_header)
except http.cookies.CookieError:
    C = http.cookies.SimpleCookie()

# 2. Check for existing session ID and visit count
if SESSION_COOKIE_NAME in C:
    session_id = C[SESSION_COOKIE_NAME].value

if VISIT_COUNT_COOKIE_NAME in C:
    try:
        visit_count = int(C[VISIT_COUNT_COOKIE_NAME].value)
    except ValueError:
        visit_count = 0 # Reset if invalid value found

# 3. Create or update session data
if not session_id:
    # New Session: Generate a unique ID (UUID4 is standard)
    session_id = str(uuid.uuid4())
    status_message = "New session created."
else:
    status_message = "Existing session found."

# 4. Increment the visit count
visit_count += 1

# --- Prepare Headers ---

# Set the new/updated Session ID cookie
C[SESSION_COOKIE_NAME] = session_id
C[SESSION_COOKIE_NAME]["max-age"] = MAX_AGE_SECONDS
C[SESSION_COOKIE_NAME]["path"] = "/"
C[SESSION_COOKIE_NAME]["httponly"] = True

# Set the updated Visit Count cookie
C[VISIT_COUNT_COOKIE_NAME] = visit_count
C[VISIT_COUNT_COOKIE_NAME]["max-age"] = MAX_AGE_SECONDS
C[VISIT_COUNT_COOKIE_NAME]["path"] = "/"

# --- Output CGI Headers ---
sys.stdout.write("Content-Type: text/plain\r\n")

# Write the Set-Cookie header(s)
for morsel in C.values():
    sys.stdout.write(f"Set-Cookie: {morsel.OutputString()}\r\n")

sys.stdout.write("\r\n")

# --- Output Body ---
sys.stdout.write("--- SESSION ID TESTER ---\n")
sys.stdout.write(f"Server Timestamp: {time.strftime('%Y-%m-%d %H:%M:%S', time.gmtime())} UTC\n\n")
sys.stdout.write(f"Status: {status_message}\n")
sys.stdout.write(f"Request Method: {method}\n")
sys.stdout.write(f"Incoming Cookie Header: {incoming_cookie_header}\n")
sys.stdout.write(f"\n--- Current Session State ---\n")
sys.stdout.write(f"-> Session ID: {session_id}\n")
sys.stdout.write(f"-> Visit Count: {visit_count}\n")
sys.stdout.write(f"-> Session Timeout (Max-Age): {MAX_AGE_SECONDS} seconds\n")
sys.stdout.write("\nReload this page multiple times to see the visit count increase and the same Session ID reused.\n")
sys.stdout.write("--- END TEST ---\n")
