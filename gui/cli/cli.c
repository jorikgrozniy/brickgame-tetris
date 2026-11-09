#include <dlfcn.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "brickgame.h"
#include "defines_cli.h"

#define LIB_TETRIS "libtetris"

#ifdef __linux__
#define LIB_EXT ".so"
#elif __APPLE__
#define LIB_EXT ".dylib"
#endif

bool loadLibrary(const char* lib_name,
                 void (**userInput_ptr)(UserAction_t action, bool hold),
                 GameInfo_t (**updateCurrentState_ptr)()) {
  static void* handle = NULL;
  if (handle) {
    dlclose(handle);
    *userInput_ptr = NULL;
    *updateCurrentState_ptr = NULL;
  }
  if (lib_name) {
    const char* paths[] = {"lib/", "../lib/", "./", "brick_game/tetris/lib/",
                           NULL};
    char path[64], lib[24];
    strcpy(lib, lib_name);
    strcat(lib, LIB_EXT);
    for (int i = 0; paths[i] != NULL; i++) {
      if (i == 0 || access(path, F_OK) != 0) {
        strcpy(path, paths[i]);
        strcat(path, lib);
      }
    }
    if (access(path, F_OK) != 0) {
      strcpy(path, lib);
    }

    if (access(path, F_OK) == 0 || strchr(path, '/') == NULL) {
      handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
      if (handle) {
        void* void_ptr = dlsym(handle, "userInput");
        memcpy(userInput_ptr, &void_ptr, sizeof(void_ptr));
        void_ptr = dlsym(handle, "updateCurrentState");
        memcpy(updateCurrentState_ptr, &void_ptr, sizeof(void_ptr));
      }
    }
  } else
    handle = NULL;
  return handle;
}

int get_user_action(UserAction_t* action, bool* hold) {
  *action = Up;
  *hold = false;
  int input_made = 1;
  Keys_t ch = getch();
  switch (ch) {
    case KEY_START:
      *action = Start;
      break;
    case KEY_PAUSE:
      *action = Pause;
      break;
    case KEY_TERMINATE:
      *action = Terminate;
      break;
    case KEY_MOVELEFT:
      *action = Left;
      break;
    case KEY_MOVERIGHT:
      *action = Right;
      break;
    case KEY_MOVEUP:
      *action = Up;
      break;
    case KEY_MOVEDOWN:
      *action = Down;
      break;
    case KEY_ACTION:
      *action = Action;
      break;
    default:
      input_made = 0;
      break;
  }
  return input_made;
}

WINDOW* initNcurses() {
  WINDOW* stdscr_local = initscr();
  keypad(stdscr_local, TRUE);
  curs_set(0);
  noecho();
  timeout(0);
  return stdscr_local;
}

void draw_window(int width, int height) {
  for (int y = 1; y <= height; y++) {
    mvaddch(y, 0, '|');
    mvaddch(y, width + 1, '|');
  }
  for (int x = 1; x <= width; x++) {
    mvaddch(0, x, '-');
    mvaddch(height + 1, x, '-');
  }
  mvaddch(0, 0, '+');
  mvaddch(0, width + 1, '+');
  mvaddch(height + 1, 0, '+');
  mvaddch(height + 1, width + 1, '+');
  refresh();
}

void draw_info_plates(int start_x, int start_y, int width, int height,
                      const char* game) {
  for (int y = 1; y <= height; y++) {
    mvaddch(start_y + y, start_x + width + 1, '|');
  }
  mvprintw(start_y + 1, start_x + width + 5, "%s", game);
  mvprintw(start_y + 3, start_x + width + 3, "High score");
  mvprintw(start_y + 6, start_x + width + 3, "Score");
  mvprintw(start_y + 9, start_x + width + 3, "Level");
  mvprintw(start_y + 12, start_x + width + 3, "Speed");
  mvprintw(start_y + 15, start_x + width + 3, "Next");
  refresh();
}

int is_field_null(int** matrix, int rows, int cols) {
  int result = 1;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      if (matrix[i][j]) result = 0;
    }
  }
  return result;
}

void draw_state(GameInfo_t info, int start_x, int start_y, int width,
                int height) {
  for (int i = 3; i <= 9; i++) {
    mvprintw(start_y + i, start_x + width + 20,
             "                                   ");
  }
  if (is_field_null(info.field, height, width)) {
    mvprintw(start_y + 19, start_x + width + 3, "PRESS ENTER");
    mvprintw(start_y + 20, start_x + width + 4, "TO START");
    mvprintw(start_y + 3, start_x + width + 20, "Controls:");
    mvprintw(start_y + 4, start_x + width + 20, "Enter - start");
    mvprintw(start_y + 5, start_x + width + 20, "P - pause");
    mvprintw(start_y + 6, start_x + width + 20, "Q - exit");
    mvprintw(start_y + 7, start_x + width + 20,
             "Left/right arrow - move left/right");
    mvprintw(start_y + 8, start_x + width + 20, "Down arrow - fall");
    mvprintw(start_y + 9, start_x + width + 20, "Space - rotate");
  } else if (info.pause)
    mvprintw(start_y + 19, start_x + width + 5, "PAUSED");
  else {
    mvprintw(start_y + 19, start_x + width + 3, "           ");
    mvprintw(start_y + 20, start_x + width + 4, "        ");
  }
}

void draw_matrix(int** matrix, int rows, int cols, int x, int y, int mode) {
  int pos_y = y + 1;
  int init_row = mode == MODE_FIGURE ? 1 : 0;
  for (int i = init_row; i < rows; i++) {
    int pos_x = x + 1;
    for (int j = 0; j < cols; j++) {
      if (mode == MODE_FIELD)
        CLEAR_POS(pos_y, pos_x);
      else if (mode == MODE_FIGURE)
        mvaddch(pos_y, pos_x, ' ');
      if (matrix[i][j] != 0) FILL_POS(pos_y, pos_x);
      pos_x++;
    }
    pos_y++;
  }
}

void draw_game_info(GameInfo_t info, int start_x, int start_y, int width) {
  for (int i = 4; i <= 13; i += 3) {
    mvprintw(start_y + i, start_x + width + 4, "      ");
  }
  mvprintw(start_y + 4, start_x + width + 4, "%d", info.high_score);
  mvprintw(start_y + 7, start_x + width + 4, "%d", info.score);
  mvprintw(start_y + 10, start_x + width + 4, "%d", info.level);
  mvprintw(start_y + 13, start_x + width + 4, "%d", info.speed);
  draw_matrix(info.next, 3, FIGURE_MAX_LENGTH, start_x + width + 3,
              start_y + 15, MODE_FIGURE);
}

void draw_frame(GameInfo_t info) {
  draw_matrix(info.field, FIELD_H, FIELD_W, OFFSET_X, OFFSET_Y, MODE_FIELD);
  draw_game_info(info, OFFSET_X, OFFSET_Y, FIELD_W);
  draw_state(info, OFFSET_X, OFFSET_Y, FIELD_W, FIELD_H);
  refresh();
}

void free_snapshot(GameInfo_t* snapshot) {
  for (int i = 0; i < FIELD_H; i++) {
    free(snapshot->field[i]);
  }
  free(snapshot->field);

  for (int i = 0; i < FIGURE_MAX_LENGTH; i++) {
    free(snapshot->next[i]);
  }
  free(snapshot->next);
}

int main(int argc, char* argv[]) {
  const char* game_title;
  const char* game_lib;
  if (argc != 2) {
    printf("Usage: ./brickgame <game>\nGames available: tetris\n");
    return 0;
  } else {
    if (strcmp(argv[1], "tetris") == 0) {
      game_title = "TETRIS";
      game_lib = LIB_TETRIS;
    } else {
      printf("No game \"%s\" found\n", argv[1]);
      return 1;
    }
  }

  void (*userInput_ptr)(UserAction_t action, bool hold) = NULL;
  GameInfo_t (*updateCurrentState_ptr)() = NULL;
  if (!loadLibrary(game_lib, &userInput_ptr, &updateCurrentState_ptr) ||
      !userInput_ptr || !updateCurrentState_ptr) {
    printf("Error loading library!\n");
    return 1;
  }

  initNcurses();
  draw_window(WINDOW_W, WINDOW_H);
  draw_info_plates(OFFSET_X, OFFSET_Y, FIELD_W, FIELD_H, game_title);

  while (1) {
    UserAction_t action = Up;
    bool hold = false;
    if (get_user_action(&action, &hold)) userInput_ptr(action, hold);
    GameInfo_t game_info = updateCurrentState_ptr();
    if (action == Terminate) break;
    draw_frame(game_info);
    napms(FRAME_DELAY);
    free_snapshot(&game_info);
  }

  loadLibrary(NULL, &userInput_ptr, &updateCurrentState_ptr);
  endwin();
  return 0;
}