#!/usr/bin/env python3

import sys
import os

# Extract query string parameter 'name'
query_string = os.environ.get('QUERY_STRING', '')
name = ''

# Parse query string for name parameter
if query_string:
    params = query_string.split('&')
    for param in params:
        if '=' in param:
            key, value = param.split('=', 1)
            if key == 'name':
                name = value
                break

# Read request body from stdin
body = f"<h1>Hello {name}!</h1>"

# Output HTTP headers
print("Content-Type: text/plain", end='\r\n')
print(f"Content-Length: {len(body)}", end='\r\n')
print("", end='\r\n')

# Output body
print(body, end='')