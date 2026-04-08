#!/bin/bash
# run_tests.sh
# Run `make run` in multiple directories and exit immediately if any test fails


NC='\e[0m' # No Color
RED='\e[0;31m'
GREEN='\e[0;32m'
YELLOW='\e[1;33m'
BLUE='\e[0;34m'
CYAN='\e[0;36m'
WHITE='\e[1;37m'

printf "%b\n" "${BLUE}Starting webserv tester...${NC}"

set -e  # Exit immediately if any command fails

# List of directories containing tests
DIRS=(
    "test/connection"
    "test/parsing/config"
    "test/parsing/HTTPRequest"
)

for dir in "${DIRS[@]}"; do
    echo "=== Running tests in $dir ==="

    if [ -d "$dir" ]; then
        cd "$dir"
        if ! make all >/dev/null 2>&1; then
            echo "Make failed in $dir"
            exit 1
        fi
		make run --no-print-directory
        if ! make fclean >/dev/null 2>&1; then
            echo "Make failed in $dir"
            exit 1
        fi
        cd - > /dev/null
    else
        echo "Warning: directory $dir does not exist, skipping."
    fi
done

printf "%b\n" "${GREEN}All tests complete!${NC}"