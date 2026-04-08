################################################################################
# Basics                                                                       #
################################################################################

NAME = webserv
CPP = $(HOME)/sgoinfre/LLVM-21.1.8-Linux-X64/bin/clang++
CPPFLAGS = -Wall -Werror -Wextra -MMD --std=c++23 --stdlib=libc++
LDFLAGS = -Wl,-rpath,$(HOME)/sgoinfre/LLVM-21.1.8-Linux-X64/lib/x86_64-unknown-linux-gnu -lc++ -lc++abi
DEBUGFLAGS = -g
ASANFLAGS = -fsanitize=address,undefined -g

################################################################################
# ENV		                                                                   #
################################################################################
PORT = 4242
################################################################################
# Source files                                                                 #
################################################################################

BUILD_DIR = build

SRC =	main.cpp \
		Server.cpp \
		HTTPRequest.cpp

INCLUDE_FLAGS = -I../mocks/incl -I../../incl/connection -I../../incl/parsing -I../../srcs/connection

OBJ = $(addprefix $(BUILD_DIR)/,$(patsubst %.cpp,%.o,$(SRC)))
DEP = $(OBJ:.o=.d)

vpath %.cpp .:../../srcs/connection
vpath %.cpp .:mocks/srcs/

################################################################################
# Rules                                                                        #
################################################################################

all: $(NAME)
	@printf "$(COLOUR_BLUE)Executable: $(NAME)\n$(COLOUR_END)"
	
run: all
	./$(NAME)

$(NAME): $(OBJ) Makefile
	@$(CPP) $(LDFLAGS) $(OBJ) -o $@
	@printf "$(COLOUR_GREEN)Compilation done👍\n$(COLOUR_END)"

$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR)
	@printf "$(COLOUR_BLUE)Compiling $< \n$(COLOUR_END)"
	@$(CPP) $(CPPFLAGS) $(INCLUDE_FLAGS) -c $< -o $@ -MF $(BUILD_DIR)/$*.d

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
# Installs                                                                     #
################################################################################

LLVM_VERSION = 21.1.8
LLVM_TAR = LLVM-$(LLVM_VERSION)-Linux-X64.tar.xz
LLVM_DIR = $(HOME)/sgoinfre/LLVM-$(LLVM_VERSION)-Linux-X64

install_cpp23:
	wget -P $(HOME)/sgoinfre https://github.com/llvm/llvm-project/releases/download/llvmorg-$(LLVM_VERSION)/$(LLVM_TAR)
	tar xf $(HOME)/sgoinfre/$(LLVM_TAR) -C $(HOME)/sgoinfre

.PHONY += install_cpp23

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