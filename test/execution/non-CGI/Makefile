################################################################################
# Basics                                                                       #
################################################################################

NAME = non-CGI_execution_test
CPP = c++
CPPFLAGS = -Wall -Werror -Wextra -MMD --std=c++23 --stdlib=libc++
LDFLAGS = -Wl,-rpath,$(HOME)/.local/lib/x86_64-unknown-linux-gnu -lc++ -lc++abi
DEBUGFLAGS = -g
ASANFLAGS = -fsanitize=address,undefined

################################################################################
# Source files                                                                 #
################################################################################

BUILD_DIR = build
ROOT_DIR := $(abspath ../../..)
DOCTEST=$(ROOT_DIR)/test/incl/doctest.h

SRC =	main.cpp \
		parseConfig.cpp \
		parseLocation.cpp \
		parseServerBlock.cpp \
		Execution.cpp \
		HTTPResponse.cpp \
		executionHelpers.cpp \
		HTTPRules.cpp \
		parsing.cpp \
		stringTrim.cpp \
		split.cpp \
		contentType.cpp \
		logging.cpp \
		loggingHelpers.cpp





INCLUDE_FLAGS = -I$(ROOT_DIR)/incl/parsing -I$(ROOT_DIR)/incl/execution -I$(ROOT_DIR)/incl/logging -I$(ROOT_DIR)/incl/connection

OBJ = $(addprefix $(BUILD_DIR)/,$(patsubst %.cpp,%.o,$(SRC)))
DEP = $(OBJ:.o=.d)

vpath %.cpp .:$(ROOT_DIR)/srcs/parsing/config
vpath %.cpp .:$(ROOT_DIR)/srcs/parsing
vpath %.cpp .:$(ROOT_DIR)/srcs/execution
vpath %.cpp .:$(ROOT_DIR)/srcs/logging



################################################################################
# Rules                                                                        #
################################################################################

all: $(NAME)
	@printf "$(COLOUR_BLUE)Executable: $(NAME)\n$(COLOUR_END)"

run: all
	@./$(NAME) -ni -nv

$(NAME): $(DOCTEST) $(OBJ) Makefile
	@$(CPP) $(LDFLAGS) $(OBJ) -o $@
	@printf "$(COLOUR_GREEN)Compilation done👍\n$(COLOUR_END)"

$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR)
	@printf "$(COLOUR_BLUE)Compiling $< \n$(COLOUR_END)"
	@$(CPP) $(CPPFLAGS) $(INCLUDE_FLAGS) -c $< -o $@ -MF $(BUILD_DIR)/$*.d

clean:
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

debug: CPPFLAGS += $(DEBUGFLAGS)
debug: re

asan: CPPFLAGS += $(ASANFLAGS) $(DEBUGFLAGS)
asan: LDFLAGS += $(ASANFLAGS)
asan: re

$(BUILD_DIR):
	mkdir -p $@

$(DOCTEST):
	mkdir -p $(ROOT_DIR)/test/incl && wget -q https://raw.githubusercontent.com/doctest/doctest/master/doctest/doctest.h -O $(ROOT_DIR)/test/incl/doctest.h && ls -lh $(ROOT_DIR)/test/incl/doctest.h

-include $(DEP)

.PHONY:	clean fclean re all run debug asan

################################################################################
# Terminal beautification                                                      #
################################################################################

# Colours
COLOUR_GREEN=\033[0;32m
COLOUR_RED=\033[0;31m
COLOUR_BLUE=\033[0;34m
COLOUR_END=\033[0m

# Formatting
FORMAT_BOLD=\033[1mBold
FORMAT_NORMAL=\033[0mNormal