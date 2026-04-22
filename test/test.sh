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
    *)
        printf "%b\n" "${YELLOW}Usage: bash test/test.sh [-m]${NC}"
        exit 2
        ;;
esac