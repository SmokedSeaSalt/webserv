.PHONY: all clean fclean re

CONNECTION_DIR := connection

all:
	$(MAKE) -C $(CONNECTION_DIR) run

install_doctest:
	mkdir -p incl && wget -q https://raw.githubusercontent.com/doctest/doctest/master/doctest/doctest.h -O incl/doctest.h && ls -lh incl/doctest.h

clean:
	$(MAKE) -C $(CONNECTION_DIR) clean

fclean:
	$(MAKE) -C $(CONNECTION_DIR) fclean

re:
	$(MAKE) -C $(CONNECTION_DIR) re

	