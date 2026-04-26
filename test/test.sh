#!/bin/bash

NC='\e[0m'
RED='\e[0;31m'
GREEN='\e[0;32m'
YELLOW='\e[1;33m'
BLUE='\e[0;34m'

set -uo pipefail

printf "%b\n" "${BLUE}Starting webserv tester...${NC}"

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
LOG_DIR="$ROOT_DIR/test/.logs"
mkdir -p "$LOG_DIR"

# Max parallel jobs in -m mode (override: JOBS=8 bash test/test.sh -m)
JOBS="${JOBS:-$(nproc)}"

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

run_end_to_end() {
    CONF="$ROOT_DIR/test/endToEnd/endToEnd.conf"

    printf "\n%b\n" "${BLUE}=== Building main webserv ===${NC}"
    cd "$ROOT_DIR" || return 1
    make || return 1

    FREE_PORT=$(python3 -c "import socket; s=socket.socket(); s.bind(('',0)); print(s.getsockname()[1]); s.close()")

    sed -i "s/listen 127.0.0.1:8080\+;/listen 127.0.0.1:${FREE_PORT};/" $CONF

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

run_one() {
    local dir="$1"
    cd "$ROOT_DIR/$dir" || return 1
    make all
    make run --no-print-directory
    make fclean >/dev/null 2>&1
}

run_multi() {
    declare -A PID_TO_DIR
    declare -A PID_TO_LOG
    PIDS=()

    for dir in "${DIRS[@]}"; do
        if [[ ! -d "$ROOT_DIR/$dir" ]]; then
            printf "%b\n" "${YELLOW}Warning: $dir does not exist, skipping.${NC}"
            continue
        fi

        while (( "$(jobs -rp | wc -l)" >= JOBS )); do
            sleep 0.1
        done

        log="$LOG_DIR/${dir//\//_}.log"
        printf "%b\n" "${BLUE}Queued: $dir${NC}"
        (run_one "$dir") >"$log" 2>&1 &
        pid=$!
        PIDS+=("$pid")
        PID_TO_DIR["$pid"]="$dir"
        PID_TO_LOG["$pid"]="$log"
    done

    FAILED=0

    for pid in "${PIDS[@]}"; do
        dir="${PID_TO_DIR[$pid]}"
        log="${PID_TO_LOG[$pid]}"

        if wait "$pid"; then
            status_color="$GREEN"
            status_text="PASS"
        else
            status_color="$RED"
            status_text="FAIL"
            FAILED=1
        fi

        # Print full output per case, one block at a time (no interleaving)
        printf "\n%b\n" "${BLUE}===== $dir =====${NC}"
        printf "%b\n" "${status_color}Result: ${status_text}${NC}"
        printf "%b\n" "${YELLOW}--- output ---${NC}"
        cat "$log" || true
        printf "%b\n" "${YELLOW}--- end output ---${NC}"
    done

    if (( FAILED )); then
        printf "%b\n" "${RED}Some tests failed.${NC}"
        return 1
    fi

    printf "%b\n" "${GREEN}All tests complete!${NC}"
}

case "${1:-}" in
    "")
        run_single
        ;;
    -m)
        run_multi
        ;;
    -e)
        run_end_to_end
        ;;
    *)
        printf "%b\n" "${YELLOW}Usage: bash test/test.sh [-m]${NC}"
        exit 2
        ;;
esac

