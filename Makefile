################################################################################
# Basics                                                                       #
################################################################################

NAME = webserv
CPP = c++
CPPFLAGS = -Wall -Werror -Wextra -MMD --std=c++23 --stdlib=libc++
LDFLAGS = -Wl,-rpath,$(HOME)/.local/lib/x86_64-unknown-linux-gnu -lc++ -lc++abi
DEBUGFLAGS = -g
ASANFLAGS = -fsanitize=address,undefined

################################################################################
# Source files                                                                 #
################################################################################

BUILD_DIR = build

ROOT_DIR := $(abspath .)

SRC =	main.cpp \
		parseConfig.cpp \
		parseLocation.cpp \
		parseServerBlock.cpp \
		Execution.cpp \
		HTTPResponse.cpp \
		HTTPRequest.cpp \
		executionHelpers.cpp \
		HTTPRules.cpp \
		parsing.cpp \
		stringTrim.cpp \
		split.cpp \
		contentType.cpp \
		Server.cpp \
		ConnectionManager.cpp \
		connectionHelpers.cpp \
		InputArgs.cpp \
		logging.cpp \
		loggingHelpers.cpp \
		configUtils.cpp \
		Client.cpp \
		Cgi.cpp

INCLUDE_FLAGS = -I$(ROOT_DIR)/incl/parsing -I$(ROOT_DIR)/incl/execution -I$(ROOT_DIR)/incl/connection  -I$(ROOT_DIR)/incl/logging#TODO

OBJ = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRC))
DEP = ${OBJ:.o=.d}

vpath %.cpp .:$(ROOT_DIR)/srcs/parsing #TODO
vpath %.cpp .:$(ROOT_DIR)/srcs/parsing/config
vpath %.cpp .:$(ROOT_DIR)/srcs/logging
vpath %.cpp .:$(ROOT_DIR)/srcs/connection
vpath %.cpp .:$(ROOT_DIR)/srcs/execution
vpath %.cpp .:$(ROOT_DIR)/srcs





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

format:
	git ls-files '*.cpp' '*.hpp' | xargs -r clang-format -i

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

-include $(DEP)

.PHONY:	clean fclean re all run debug asan test format

################################################################################
# Testing                                                                      #
################################################################################

test:
	@./test/test.sh

test_multithread:
	@./test/test.sh -m

test_end_to_end:
	@./test/test.sh -e



.PHONY += test

################################################################################
# Installs                                                                     #
################################################################################

LLVM_VERSION = 21.1.8
LLVM_TAR = LLVM-$(LLVM_VERSION)-Linux-X64.tar.xz
LLVM_DIR = $(HOME)/sgoinfre/LLVM-$(LLVM_VERSION)-Linux-X64

install_cpp23: download_cpp23 move_cpp23_to_home add_local_cpp23_to_path

download_cpp23:
	wget -P $(HOME)/sgoinfre https://github.com/llvm/llvm-project/releases/download/llvmorg-$(LLVM_VERSION)/$(LLVM_TAR)
	tar -xf $(HOME)/sgoinfre/$(LLVM_TAR) -C $(HOME)/sgoinfre

move_cpp23_to_home:
	# Create target directories if they don't exist
	mkdir -p $(HOME)/.local/lib/x86_64-unknown-linux-gnu
	mkdir -p $(HOME)/.local/bin

	# Copy the libraries
	cp -r $(LLVM_DIR)/lib/x86_64-unknown-linux-gnu/* \
	      $(HOME)/.local/lib/x86_64-unknown-linux-gnu/

	# Create/update symlink for c++
	ln -sf $(LLVM_DIR)/bin/clang++ \
	       $(HOME)/.local/bin/c++

add_local_cpp23_to_path:
	# Ensure ~/.local/bin is at the front of PATH in bashrc
	grep -qxF 'export PATH="$(HOME)/.local/bin:$$PATH"' $(HOME)/.bashrc || \
	echo 'export PATH="$(HOME)/.local/bin:$$PATH"' >> $(HOME)/.bashrc

	# Ensure ~/.local/bin is at the front of PATH in zshrc
	grep -qxF 'export PATH="$(HOME)/.local/bin:$$PATH"' $(HOME)/.zshrc || \
	echo 'export PATH="$(HOME)/.local/bin:$$PATH"' >> $(HOME)/.zshrc

	@printf "$(COLOUR_GREEN)Make sure to source your shell config file \
	for the changes in path to register!\n$(COLOUR_END)"

.PHONY += install_cpp23 download_cpp23 move_cpp23_to_home add_local_cpp23_to_path

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