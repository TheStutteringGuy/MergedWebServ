#!/usr/bin/env python3
import os
import http.cookies
import sys

# Define the cookie name for our session-like counter
COOKIE_NAME = "page_visits"
# Define the maximum age for the cookie (e.g., 3600 seconds = 1 hour)
# Set to -1 or 0 for a session-only cookie that expires when the browser closes.
MAX_AGE = 3600 

def get_cookie_value(cookie_jar, name):
    """Safely get a cookie value, converting it to an integer."""
    if name in cookie_jar:
        try:
            return int(cookie_jar[name].value)
        except ValueError:
            # Handle case where cookie value is not a valid integer
            return 0
    return 0

def main():
    """Main function to handle the CGI logic."""
    
    # 1. Initialize a cookie jar and load existing cookies from the environment
    cookie_jar = http.cookies.SimpleCookie()
    
    # Check if the HTTP_COOKIE environment variable exists
    if 'HTTP_COOKIE' in os.environ:
        try:
            cookie_data = os.environ['HTTP_COOKIE']
            cookie_jar.load(cookie_data)
        except Exception as e:
            # Print to stderr for server log debugging
            print(f"Error loading cookies: {e}", file=sys.stderr)

    # 2. Get the current visit count
    current_visits = get_cookie_value(cookie_jar, COOKIE_NAME)
    
    # 3. Increment the count for the current visit
    new_visits = current_visits + 1

    # 4. Set the new cookie for the response
    new_cookie = http.cookies.SimpleCookie()
    new_cookie[COOKIE_NAME] = str(new_visits)
    # Set the cookie properties
    new_cookie[COOKIE_NAME]["path"] = "/"
    new_cookie[COOKIE_NAME]["max-age"] = MAX_AGE 
    new_cookie[COOKIE_NAME]["httponly"] = True # Recommended for security
    
    # 5. Print HTTP headers
    print("Content-Type: text/html")
    # Print the Set-Cookie header
    print(new_cookie.output(header=''))
    print("") # End of headers
    
    # 6. Print HTML content
    print(f"<html>")
    print(f"<head><title>Session Test</title></head>")
    print(f"<body>")
    print(f"<h1>CGI Session Test Success! 🎉</h1>")
    print(f"<p>The counter is tracking your visits using a cookie.</p>")
    print(f"<h2>You have visited this page **{new_visits}** time(s).</h2>")
    
    if new_visits == 1:
        print("<p>Refresh the page to see the counter increase!</p>")
    else:
        print(f"<p>The previous count was **{current_visits}**.</p>")

    print(f"<hr>")
    print(f"<h3>Debug Info (Sent Cookies):</h3>")
    print(f"<p>HTTP_COOKIE Environment Variable: <code>{os.environ.get('HTTP_COOKIE', 'N/A')}</code></p>")
    print(f"<h3>Debug Info (New Cookie Header):</h3>")
    # Use output() again just to show the generated header content in the body
    print(f"<pre>{new_cookie.output()}</pre>") 
    
    print(f"</body>")
    print(f"</html>")

if __name__ == "__main__":
    main()

print("Hello")