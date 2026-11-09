#ifndef DEFINES_CLI_H
#define DEFINES_CLI_H

typedef enum {
  KEY_START = 10,
  KEY_PAUSE = 112,
  KEY_TERMINATE = 113,
  KEY_MOVELEFT = 260,
  KEY_MOVERIGHT = 261,
  KEY_MOVEUP = 259,
  KEY_MOVEDOWN = 258,
  KEY_ACTION = 32
} Keys_t;

#define CHAR_FILLED '#'
#define CHAR_BLANK '.'

#define MODE_FIELD 0
#define MODE_FIGURE 1

#define WINDOW_H 20
#define WINDOW_W 24

#define OFFSET_X 0
#define OFFSET_Y 0

#define FILL_POS(y, x) mvaddch(y, x, CHAR_FILLED)
#define CLEAR_POS(y, x) mvaddch(y, x, CHAR_BLANK)

#endif