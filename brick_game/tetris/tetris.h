#ifndef TETRIS_H
#define TETRIS_H

#include <stdio.h>
#include <sys/time.h>

#include "brickgame.h"
#include "defines_tetris.h"

typedef enum {
  ZERO_POINTS = 0,
  ROWS_1 = 100,
  ROWS_2 = 300,
  ROWS_3 = 700,
  ROWS_4 = 1500
} Points_t;

typedef enum { PLACE, REMOVE, CHECK } MatrixAction_t;

typedef enum {
  START,
  SPAWN,
  MOVING,
  SHIFTING,
  ATTACHING,
  GAMEOVER
} GameState_t;

typedef struct {
  int** matrix;
  int x;
  int y;
  int rotation;
  bool init_place;
} Figure_t;

typedef struct {
  GameInfo_t game_info;
  Figure_t current_figure;
  GameState_t state;
  UserAction_t last_action;
  bool input_made;
  struct timespec last_shift;
  long shift_interval;
  bool initialized;
  bool terminated;
} Game_t;

typedef void (*action)(void);

Game_t* staticGame();
action fsm_get_action();
GameInfo_t create_snapshot(const GameInfo_t* src);

int is_state_indep();
long update_time(bool force);
void spawn();
void moveleft();
void moveright();
void movedown();
void shift();
void attach();
void rotate();
void gameover();
void terminate();
void pause_game();

void init_game();
void init_stats();
void init_field();
void init_figure();
void free_game_memory();
void add_score(int amount);
void save_high_score();
int load_high_score();
void set_level();
void set_speed();
void update_figure();
void place_current_figure();
void remove_placed_figure();
void fall_figure();
void move_figure(int direction);
int shift_filled_lines();
void get_random_figure(int** dst);

int check_overflow();
int check_collision_down();
int check_collision_left();
int check_collision_right();

#endif