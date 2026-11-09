# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -D_DEFAULT_SOURCE -g $(INC_FLAGS)
LDFLAGS = -lncurses
GFLAGS = -fprofile-arcs -ftest-coverage
CHKFLAGS = -lcheck -lm -lpthread

# Installation directories
PREFIX = /brick_game_v1
LIBDIR = $(PREFIX)/lib
DATADIR = $(PREFIX)/share
DOCDIR = $(DATADIR)/doc

# Custom installation directory
DESTDIR = /Users/jorikgrozniy/Documents

# Source files
SRC_LIB = brick_game/tetris/tetris.c brick_game/tetris/utils/utils_matrix.c
SRC_CLI = gui/cli/cli.c
OBJ_LIB = $(SRC_LIB:%.c=$(OBJ_DIR)/%.o)
OBJ_CLI = $(SRC_CLI:%.c=$(OBJ_DIR)/%.o)
EXE_NAME = brickgame
LIB_NAME = libtetris

# Directories
LIB_DIR = brick_game/tetris/lib
OBJ_DIR = obj
EXE_DIR = .
TEST_DIR = brick_game/tetris/test
DIST_DIR = dist
DOC_DIR = doc

HEADERS = brick_game/include/brickgame.h brick_game/tetris/defines_tetris.h brick_game/tetris/tetris.h brick_game/tetris/utils/utils.h gui/cli/defines_cli.h
INC_DIRS = brick_game/include brick_game/tetris brick_game/tetris/utils gui/cli
INC_FLAGS = $(addprefix -I,$(INC_DIRS))

# Platform detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    LIB_EXT = .dylib
    LIB_FLAGS = -dynamiclib -fPIC
else
    LIB_EXT = .so
    LIB_FLAGS = -shared -fPIC
	LDFLAGS += -ldl -Wl,-rpath,.
    CFLAGS += -D_POSIX_C_SOURCE=199309L
    CHKFLAGS += -lsubunit
endif

LIB = $(LIB_DIR)/$(LIB_NAME)$(LIB_EXT)
EXE = $(EXE_DIR)/$(EXE_NAME)
STATIC_LIB = $(LIB_DIR)/$(LIB_NAME).a

.PHONY: all
all: clean $(LIB) $(EXE)

$(STATIC_LIB): $(OBJ_LIB)
	@mkdir -p $(LIB_DIR)
	ar rcs $@ $^

$(LIB): $(OBJ_LIB)
	@mkdir -p $(LIB_DIR)
	$(CC) $(LIB_FLAGS) -o $@ $^

$(EXE): $(OBJ_CLI)
	@mkdir -p $(EXE_DIR)
	$(CC) -o $@ $(OBJ_CLI) $(LDFLAGS)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

.PHONY: test
test: clean
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(LIB_DIR)
	echo "0" > tetris.score

	$(CC) $(CFLAGS) $(GFLAGS) -c brick_game/tetris/tetris.c -o $(OBJ_DIR)/tetris.o
	$(CC) $(CFLAGS) $(GFLAGS) -c brick_game/tetris/utils/utils_matrix.c -o $(OBJ_DIR)/utils_matrix.o
	ar rcs $(LIB_DIR)/$(LIB_NAME).a $(OBJ_DIR)/*.o

	$(CC) $(CFLAGS) -c $(TEST_DIR)/test.c -o $(OBJ_DIR)/test.o
	$(CC) $(CFLAGS) $(GFLAGS) $(OBJ_DIR)/test.o $(LIB_DIR)/$(LIB_NAME).a -o test $(CHKFLAGS)
	./test

.PHONY: gcov_report
gcov_report: test
	gcov -o brick_game/tetris/tetris.c brick_game/tetris/utils/utils_matrix.c
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory lcov-report
	open lcov-report/index.html || xdg-open lcov-report/index.html

.PHONY: install
install: all
	install -d $(DESTDIR)$(PREFIX)
	install -m 755 $(EXE) $(DESTDIR)$(PREFIX)/$(EXE_NAME)
	
	install -d $(DESTDIR)$(LIBDIR)
	install -m 644 $(LIB) $(DESTDIR)$(LIBDIR)/

	install -d $(DESTDIR)$(DATADIR)
	
	@echo "Brickgame v1.0 installed to $(DESTDIR)$(PREFIX)"

.PHONY: uninstall
uninstall:
	rm -rf $(DESTDIR)$(PREFIX)
	@echo "Brickgame v1.0 uninstalled from $(DESTDIR)$(PREFIX)"

.PHONY: dvi
dvi:
	@if command -v latex >/dev/null 2>&1; then \
		if [ -f $(DOC_DIR)/brickgame.tex ]; then \
			cd $(DOC_DIR) && latex brickgame.tex && latex brickgame.tex; \
			echo "DVI documentation generated in $(DOC_DIR)/brickgame.dvi"; \
		else \
			echo "No LaTeX documentation found in $(DOC_DIR)/"; \
			echo "Creating minimal documentation..."; \
			create_minimal_doc; \
		fi \
	else \
		echo "LaTeX not found. Please install texlive to generate DVI documentation."; \
	fi

.PHONY: dist
dist: clean
	@mkdir -p $(DIST_DIR)
	@PACKAGE_NAME="brickgame-$$(date +%Y%m%d)"; \
	mkdir -p $(DIST_DIR)/$$PACKAGE_NAME; \
	cp -r Makefile README.md brick_game gui $(DOC_DIR) $(DIST_DIR)/$$PACKAGE_NAME/ 2>/dev/null || true; \
	cd $(DIST_DIR) && tar -czf $$PACKAGE_NAME.tar.gz $$PACKAGE_NAME; \
	rm -rf $(DIST_DIR)/$$PACKAGE_NAME; \
	echo "Distribution package created: $(DIST_DIR)/$$PACKAGE_NAME.tar.gz"

.PHONY: clang
clang:
	clang-format -i $(SRC_LIB) $(SRC_CLI) $(HEADERS) $(TEST_DIR)/test.c

.PHONY: valgrind
valgrind: all
	valgrind --tool=memcheck --leak-check=yes ./$(EXE_NAME) tetris

.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(EXE) $(LIB_DIR) lcov-report coverage.info test
	find . -type f \( -name "*.gcno" -o -name "*.gcda" -o -name "*.gcov" -o -name "*.o" \) -delete

.PHONY: distclean
distclean: clean
	rm -rf $(DIST_DIR)
