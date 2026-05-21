#!/bin/bash

# Convert to uppercase
output=$(cat | awk '{ print toupper($0) }')

# Output HTTP headers
echo -n "Content-Type: text/plain\\r\\n"
echo -n "Content-Length: $(echo -n "$output" | wc -c)\\r\\n\\r\\n"

# Output body
echo -n "$output"