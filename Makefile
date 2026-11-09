CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror -g
CPPFLAGS ?=
LDFLAGS ?=
LDLIBS ?= -lncurses
COVERAGE_FLAGS ?= -fprofile-arcs -ftest-coverage

SRC_LIB := brick_game/tetris/tetris.c \
           brick_game/tetris/utils/utils_matrix.c
SRC_CLI := gui/cli/cli.c
HEADERS := brick_game/include/brickgame.h \
           brick_game/tetris/defines_tetris.h \
           brick_game/tetris/tetris.h \
           brick_game/tetris/utils/utils.h \
           gui/cli/defines_cli.h
INC_DIRS := brick_game/include brick_game/tetris brick_game/tetris/utils gui/cli

OBJ_DIR := obj
TEST_OBJ_DIR := $(OBJ_DIR)/test
LIB_DIR := brick_game/tetris/lib
TEST_DIR := brick_game/tetris/test
COVERAGE_DIR := coverage
DIST_DIR := dist
EXE := brickgame
LIB_NAME := libtetris
TEST_BIN := test_runner

OBJ_LIB := $(SRC_LIB:%.c=$(OBJ_DIR)/%.o)
OBJ_CLI := $(SRC_CLI:%.c=$(OBJ_DIR)/%.o)
TEST_OBJ_LIB := $(SRC_LIB:%.c=$(TEST_OBJ_DIR)/%.o)
TEST_OBJ := $(TEST_OBJ_DIR)/$(TEST_DIR)/test.o

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  LIB_EXT := dylib
  LIB_FLAGS := -dynamiclib -fPIC
  EXE_RPATH := -Wl,-rpath,@loader_path -Wl,-rpath,@loader_path/../lib -Wl,-rpath,@loader_path/brick_game/tetris/lib
  CHECK_LIBS ?= -lcheck -lm -lpthread
else
  LIB_EXT := so
  LIB_FLAGS := -shared -fPIC
  EXE_RPATH := -Wl,-rpath,'$$ORIGIN' -Wl,-rpath,'$$ORIGIN/../lib' -Wl,-rpath,'$$ORIGIN/brick_game/tetris/lib'
  LDLIBS += -ldl
  CPPFLAGS += -D_POSIX_C_SOURCE=199309L
  CHECK_LIBS ?= -lcheck -lsubunit -lm -lpthread
endif

CPPFLAGS += -D_DEFAULT_SOURCE $(addprefix -I,$(INC_DIRS))
LIB := $(LIB_DIR)/$(LIB_NAME).$(LIB_EXT)

PREFIX ?= $(HOME)/.local
DESTDIR ?=
BINDIR := $(PREFIX)/bin
LIBDIR := $(PREFIX)/lib

.PHONY: all run clean distclean test gcov_report install uninstall dist clang valgrind

all: $(LIB) $(EXE)

$(LIB): $(OBJ_LIB)
	@mkdir -p $(dir $@)
	$(CC) $(LIB_FLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(EXE): $(OBJ_CLI) $(LIB)
	$(CC) $(LDFLAGS) $(EXE_RPATH) -o $@ $(OBJ_CLI) $(LDLIBS)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -c -o $@ $<

run: all
	./$(EXE) tetris

$(TEST_BIN): $(TEST_OBJ_LIB) $(TEST_OBJ)
	$(CC) $(CFLAGS) $(COVERAGE_FLAGS) -o $@ $^ $(LDFLAGS) $(CHECK_LIBS)

$(TEST_OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(COVERAGE_FLAGS) -c -o $@ $<

test: $(TEST_BIN)
	@set -e; \
	if [ -f tetris.score ]; then mv tetris.score tetris.score.bak; fi; \
	trap 'rm -f tetris.score; if [ -f tetris.score.bak ]; then mv tetris.score.bak tetris.score; fi' EXIT; \
	./$(TEST_BIN)

gcov_report: test
	@mkdir -p $(COVERAGE_DIR)
	lcov --capture --directory $(TEST_OBJ_DIR) --output-file $(COVERAGE_DIR)/coverage.info
	lcov --remove $(COVERAGE_DIR)/coverage.info '$(TEST_DIR)/*' --output-file $(COVERAGE_DIR)/coverage.info
	genhtml $(COVERAGE_DIR)/coverage.info --output-directory $(COVERAGE_DIR)/html
	@echo "Coverage report: $(COVERAGE_DIR)/html/index.html"
	@if command -v xdg-open >/dev/null 2>&1; then \
		xdg-open $(COVERAGE_DIR)/html/index.html >/dev/null 2>&1 || true; \
	elif command -v open >/dev/null 2>&1; then \
		open $(COVERAGE_DIR)/html/index.html >/dev/null 2>&1 || true; \
	fi

install: all
	install -d $(DESTDIR)$(BINDIR) $(DESTDIR)$(LIBDIR)
	install -m 755 $(EXE) $(DESTDIR)$(BINDIR)/$(EXE)
	install -m 644 $(LIB) $(DESTDIR)$(LIBDIR)/$(notdir $(LIB))
	@echo "Installed BrickGame to $(DESTDIR)$(PREFIX)"

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(EXE)
	rm -f $(DESTDIR)$(LIBDIR)/$(notdir $(LIB))
	@echo "Removed BrickGame from $(DESTDIR)$(PREFIX)"

dist: clean
	@mkdir -p $(DIST_DIR)
	@PACKAGE=brickgame-$$(date +%Y%m%d); \
	mkdir -p $(DIST_DIR)/$$PACKAGE; \
	cp -R Makefile README.md .clang-format brick_game gui $(DIST_DIR)/$$PACKAGE/; \
	tar -czf $(DIST_DIR)/$$PACKAGE.tar.gz -C $(DIST_DIR) $$PACKAGE; \
	rm -rf $(DIST_DIR)/$$PACKAGE; \
	echo "Created $(DIST_DIR)/$$PACKAGE.tar.gz"

clang:
	clang-format -i $(SRC_LIB) $(SRC_CLI) $(HEADERS) $(TEST_DIR)/test.c

valgrind: all
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./$(EXE) tetris

clean:
	rm -rf $(OBJ_DIR) $(EXE) $(LIB_DIR) $(COVERAGE_DIR) $(TEST_BIN)
	find . -type f \( -name "*.gcno" -o -name "*.gcda" -o -name "*.gcov" -o -name "*.o" \) -exec rm -f {} +

distclean: clean
	rm -rf $(DIST_DIR)
