#!/bin/bash

NC='\e[0m'
RED='\e[0;31m'
GREEN='\e[0;32m'
YELLOW='\e[1;33m'
BLUE='\e[0;34m'

set -uo pipefail

printf "%b\n" "${BLUE}Starting webserv tester...${NC}"

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
export ROOT_DIR
LOG_DIR="$ROOT_DIR/test/.logs"
mkdir -p "$LOG_DIR"

DIRS=(
    "test/connection"
    "test/parsing/config"
    "test/parsing/HTTPRequest"
    "test/parsing/HTTPResponse"
    "test/logging"
    "test/execution/non-CGI"
)

run_single() {
    for dir in "${DIRS[@]}"; do
        printf "\n%b\n" "${BLUE}=== Running tests in $dir ===${NC}"

        if [[ ! -d "$ROOT_DIR/$dir" ]]; then
            printf "%b\n" "${YELLOW}Warning: $dir does not exist, skipping.${NC}"
            continue
        fi

        cd "$ROOT_DIR/$dir" || return 1

        make all || return 1
        make run --no-print-directory || return 1
        make fclean >/dev/null 2>&1 || return 1
    done

    printf "%b\n" "${GREEN}All tests complete!${NC}"
}

run_one() {
    local dir="$1"
    if [[ ! -d "$ROOT_DIR/$dir" ]]; then
        printf "%b\n" "${RED}Error: $dir does not exist.${NC}"
        return 1
    fi
    printf "\n%b\n" "${BLUE}=== Running tests in $dir ===${NC}"
    cd "$ROOT_DIR/$dir" || return 1
    make all || return 1
    make run --no-print-directory || return 1
    make fclean >/dev/null 2>&1 || return 1
    printf "%b\n" "${GREEN}Test $dir complete!${NC}"
}

run_end_to_end() {
    CONF="$ROOT_DIR/test/endToEnd/endToEnd.conf"

    printf "\n%b\n" "${BLUE}=== Building main webserv ===${NC}"
    cd "$ROOT_DIR" || return 1
    make || return 1

    FREE_PORT=$(python3 -c "import socket; s=socket.socket(); s.bind(('',0)); print(s.getsockname()[1]); s.close()")

    sed -i "s/listen 127.0.0.1:[0-9]\+;/listen 127.0.0.1:${FREE_PORT};/" $CONF
    printf "\n%b\n" "${BLUE}=== Running main webserv in background on port ${FREE_PORT} ===${NC}"
    ./webserv -d 4 -p $ROOT_DIR $CONF &
    WEBSERV_PID=$!

    for i in {1..20}; do
        if nc -z 127.0.0.1 "$FREE_PORT"; then
            break
        fi
        sleep 0.5
    done

    printf "\n%b\n" "${BLUE}=== Running end-to-end tests ===${NC}"
    cd "$ROOT_DIR/test/endToEnd" || return 1
    pwd
    FREE_PORT=$FREE_PORT go test main_test.go || { kill $WEBSERV_PID; return 1; }

    kill $WEBSERV_PID
    
    sed -i "s/listen 127.0.0.1:${FREE_PORT};/listen 127.0.0.1:8080;/" $CONF
    printf "${GREEN}End to end go tests passed!${NC}\n"
}

case "${1:-}" in
    "")
        run_single
        ;;
    -e)
        run_end_to_end
        ;;
    *)
        # Try to run a single test directory by name
        TEST_NAME="$1"
        FOUND=0
        for dir in "${DIRS[@]}"; do
            if [[ "$dir" == *"$TEST_NAME"* ]]; then
                run_one "$dir"
                FOUND=1
                break
            fi
        done
        if [[ $FOUND -eq 0 ]]; then
            printf "%b\n" "${RED}Test \"$TEST_NAME\" not found in DIRS.${NC}"
            exit 2
        fi
        ;;
esac