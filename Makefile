################################################################################
# Basics                                                                       #
################################################################################

NAME = webserv
CPP = c++
CPPFLAGS = -Wall -Werror -Wextra -MMD --std=c++23
DEBUGFLAGS = -g
ASANFLAGS = -fsanitaize=address,undefined -g

################################################################################
# Source files                                                                 #
################################################################################

BUILD_DIR = build

SRC =	main.cpp

OBJ = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRC))
DEP = ${OBJ:.o=.d}

################################################################################
# Rules                                                                        #
################################################################################

all: $(NAME)
	@printf "$(COLOUR_BLUE)Executable: $(NAME)\n$(COLOUR_END)"

$(NAME): $(OBJ) Makefile
	@$(CPP) $(CPPFLAGS) $(OBJ) -o $@
	@printf "$(COLOUR_GREEN)Compilation done👍\n$(COLOUR_END)"

$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR)
	@printf "$(COLOUR_BLUE)Compiling $@ \n$(COLOUR_END)"
	@$(CPP) $(CPPFLAGS) -c $< -o $@ -MF $(basename $@).d

clean:
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean
	$(MAKE) --no-print-directory

debug: CPPFLAGS += $(DEBUGFLAGS)
debug: re

asan: CPPFLAGS += $(ASANFLAGS)
asan: re

$(BUILD_DIR):
	mkdir -p $@

-include $(DEP)

.PHONY:	clean fclean re all debug asan

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