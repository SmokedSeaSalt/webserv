################################################################################
# Basics                                                                       #
################################################################################

NAME = HTTPResponse_test
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
		HTTPResponse.cpp \
		HTTPRules.cpp \
		parsing.cpp


INCLUDE_FLAGS = -I$(ROOT_DIR)/incl/parsing

OBJ = $(addprefix $(BUILD_DIR)/,$(patsubst %.cpp,%.o,$(SRC)))
DEP = $(OBJ:.o=.d)

# Link against central object files if LINK_ONLY is set
ifdef LINK_ONLY
  CENTRAL_BUILD_DIR = $(ROOT_DIR)/build
  CENTRAL_OBJS = $(CENTRAL_BUILD_DIR)/HTTPResponse.o \
                 $(CENTRAL_BUILD_DIR)/HTTPRules.o \
                 $(CENTRAL_BUILD_DIR)/parsing.o
  LINK_OBJS = $(CENTRAL_OBJS) $(BUILD_DIR)/main.o
else
  LINK_OBJS = $(OBJ)
endif

vpath %.cpp .:$(ROOT_DIR)/srcs/parsing

################################################################################
# Rules                                                                        #
################################################################################

all: $(NAME)
	@printf "$(COLOUR_BLUE)Executable: $(NAME)\n$(COLOUR_END)"

run: all
	@./$(NAME) -ni -nv

$(NAME): $(DOCTEST) $(LINK_OBJS) Makefile
	@$(CPP) $(LDFLAGS) $(LINK_OBJS) -o $@
	@printf "$(COLOUR_GREEN)Compilation done👍\n$(COLOUR_END)"

$(BUILD_DIR)/main.o: main.cpp Makefile | $(BUILD_DIR)
	@printf "$(COLOUR_BLUE)Compiling $< \n$(COLOUR_END)"
	@$(CPP) $(CPPFLAGS) $(INCLUDE_FLAGS) -c $< -o $@ -MF $(BUILD_DIR)/main.d

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