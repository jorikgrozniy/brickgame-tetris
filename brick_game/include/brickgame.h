#ifndef BRICKGAME_H
#define BRICKGAME_H

#include <stdbool.h>

#define FIELD_W 10
#define FIELD_H 20
#define FIGURE_MAX_LENGTH 4

#define GAME_FPS 30
#define FRAME_DELAY (1000 / GAME_FPS)

#define POINTS_FOR_LVL 600

#define MAX_INTERVAL_MS 1000
#define MIN_INTERVAL_MS 250

typedef enum {
  Start,
  Pause,
  Terminate,
  Left,
  Right,
  Up,
  Down,
  Action
} UserAction_t;

typedef struct {
  int** field;
  int** next;
  int score;
  int high_score;
  int level;
  int speed;
  int pause;
} GameInfo_t;

void userInput(UserAction_t action, bool hold);
GameInfo_t updateCurrentState();

#endif