NC='\e[0m' # No Color
RED='\e[0;31m'
GREEN='\e[0;32m'
YELLOW='\e[1;33m'
BLUE='\e[0;34m'
CYAN='\e[0;36m'
WHITE='\e[1;37m'

printf "%b\n" "${BLUE}Starting webserv tester...${NC}"

mkdir -p incl && wget -q https://raw.githubusercontent.com/doctest/doctest/master/doctest/doctest.h -O incl/doctest.h && ls -lh incl/doctest.h


# Connection
printf "%b\n" "${WHITE}Connection tests${NC}"

make -C connection
make -C parsing/config
