#include "tetris.h"

#include "utils.h"

Game_t* staticGame() {
  static Game_t data = {0};
  if (!data.initialized && !data.terminated) {
    data.initialized = true;
    init_game();
  }
  return &data;
}

void userInput(UserAction_t action, bool hold) {
  (void)hold;
  staticGame()->last_action = action;
  staticGame()->input_made = true;
}

GameInfo_t updateCurrentState() {
  Game_t* data = staticGame();
  action act = fsm_get_action();
  if ((act && (!data->game_info.pause || act == terminate)) ||
      (data->game_info.pause && act == pause_game)) {
    data->input_made = false;
    act();
  }

  if (data->state == MOVING && update_time(false) >= data->shift_interval &&
      !data->game_info.pause)
    data->state = SHIFTING;

  return !data->terminated ? create_snapshot(&data->game_info)
                           : (GameInfo_t){0};
}

GameInfo_t create_snapshot(const GameInfo_t* src) {
  GameInfo_t dst = {0};

  dst.pause = src->pause;
  dst.level = src->level;
  dst.speed = src->speed;
  dst.score = src->score;
  dst.high_score = src->high_score;

  dst.field = deep_copy_matrix(src->field, FIELD_H, FIELD_W);
  dst.next = deep_copy_matrix(src->next, FIGURE_MAX_LENGTH, FIGURE_MAX_LENGTH);
  return dst;
}

long update_time(bool force) {
  Game_t* data = staticGame();
  struct timespec current;
  clock_gettime(CLOCK_REALTIME, &current);
  long elapsed = (current.tv_sec - data->last_shift.tv_sec) * NS_IN_S +
                 (current.tv_nsec - data->last_shift.tv_nsec);

  if (elapsed >= data->shift_interval || force) {
    data->last_shift = current;
  }
  return elapsed;
}

action fsm_get_action() {
  Game_t* data = staticGame();
  static const action fsm_table[6][8] = {
      {spawn, NULL, terminate, NULL, NULL, NULL, NULL, NULL},
      {spawn, spawn, spawn, spawn, spawn, spawn, spawn, spawn},
      {NULL, pause_game, terminate, moveleft, moveright, NULL, movedown,
       rotate},
      {shift, shift, shift, shift, shift, shift, shift, shift},
      {attach, attach, attach, attach, attach, attach, attach, attach},
      {gameover, gameover, gameover, gameover, gameover, gameover, gameover,
       gameover}};
  return is_state_indep()
             ? fsm_table[data->state][0]
             : (data->input_made ? fsm_table[data->state][data->last_action]
                                 : NULL);
}

int is_state_indep() {
  int result = 0;
  GameState_t st = staticGame()->state;
  if (st == SPAWN || st == SHIFTING || st == ATTACHING || st == GAMEOVER)
    result = 1;
  return result;
}

void spawn() {
  Game_t* data = staticGame();
  update_figure();
  if (data->current_figure.init_place) {
    data->current_figure.y--;
    data->current_figure.init_place = false;
  }
  if (!check_overflow()) {
    set_level();
    set_speed();
    place_current_figure();
    update_time(true);
    data->state = MOVING;
  } else
    data->state = GAMEOVER;
}

void moveleft() {
  remove_placed_figure();
  if (!check_collision_left()) move_figure(DIR_LEFT);
  place_current_figure();
}

void moveright() {
  remove_placed_figure();
  if (!check_collision_right()) move_figure(DIR_RIGHT);
  place_current_figure();
}

void movedown() {
  staticGame()->state = SHIFTING;
  update_time(true);
}

void shift() {
  remove_placed_figure();
  if (!check_collision_down()) {
    fall_figure();
    staticGame()->state = MOVING;
  } else
    staticGame()->state = ATTACHING;
  place_current_figure();
}

void attach() {
  Game_t* data = staticGame();
  int rows_shifted = shift_filled_lines();
  Points_t points = ZERO_POINTS;
  switch (rows_shifted) {
    case 1:
      points = ROWS_1;
      break;
    case 2:
      points = ROWS_2;
      break;
    case 3:
      points = ROWS_3;
      break;
    case 4:
      points = ROWS_4;
      break;
    default:
      break;
  }
  add_score(points);
  if (data->game_info.score > data->game_info.high_score)
    data->game_info.high_score = data->game_info.score;
  data->state = SPAWN;
}

void rotate() {
  Game_t* data = staticGame();
  remove_placed_figure();
  do {
    rotate_matrix(data->current_figure.matrix, FIGURE_MAX_LENGTH);
    if (data->current_figure.rotation < 3)
      data->current_figure.rotation++;
    else
      data->current_figure.rotation = 0;
  } while (check_overflow());
  place_current_figure();
}

void gameover() {
  save_high_score();
  free_game_memory();
  staticGame();
}

void pause_game() {
  staticGame()->game_info.pause = !staticGame()->game_info.pause;
}

void terminate() {
  save_high_score();
  staticGame()->terminated = true;
  if (staticGame()->state != START) free_game_memory();
}

void init_game() {
  Game_t* data = staticGame();
  init_stats();
  init_field();
  init_figure();
  set_speed();
  data->input_made = false;
  data->terminated = false;
  data->state = START;
}

void init_stats() {
  GameInfo_t* gameInfo = &staticGame()->game_info;
  gameInfo->level = 1;
  gameInfo->score = 0;
  gameInfo->high_score = load_high_score();
  gameInfo->speed = 1;
  gameInfo->pause = 0;
}

void init_field() {
  staticGame()->game_info.field = create_matrix(FIELD_H, FIELD_W);
}

void init_figure() {
  Game_t* data = staticGame();
  data->current_figure.matrix =
      create_matrix(FIGURE_MAX_LENGTH, FIGURE_MAX_LENGTH);
  data->game_info.next = create_matrix(FIGURE_MAX_LENGTH, FIGURE_MAX_LENGTH);
  get_random_figure(data->game_info.next);
}

void free_game_memory() {
  Game_t* data = staticGame();
  free_matrix(data->game_info.field, FIELD_H);
  free_matrix(data->game_info.next, FIGURE_MAX_LENGTH);
  free_matrix(data->current_figure.matrix, FIGURE_MAX_LENGTH);
  data->initialized = false;
}

void add_score(int amount) { staticGame()->game_info.score += amount; }

void save_high_score() {
  const char* paths[] = {"share/tetris.score", "../share/tetris.score",
                         "tetris.score", NULL};
  FILE* file = NULL;
  for (int i = 0; paths[i] != NULL; i++) {
    if (file == NULL) file = fopen(paths[i], "w");
  }
  if (file != NULL) {
    fprintf(file, "%d\n", staticGame()->game_info.high_score);
    fclose(file);
  }
}

int load_high_score() {
  const char* paths[] = {"share/tetris.score", "../share/tetris.score",
                         "tetris.score", NULL};
  FILE* file = NULL;
  for (int i = 0; paths[i] != NULL; i++) {
    if (file == NULL) file = fopen(paths[i], "r");
  }
  int high_score = 0;
  if (file != NULL) {
    fscanf(file, "%d", &high_score);
    fclose(file);
  }
  return high_score;
}

void set_level() {
  Game_t* data = staticGame();
  int calculated_lvl = data->game_info.score / POINTS_FOR_LVL + 1;
  if (calculated_lvl > 10)
    data->game_info.level = 10;
  else
    data->game_info.level = calculated_lvl;
}

void set_speed() {
  Game_t* data = staticGame();
  data->game_info.speed =
      (NS_IN_MS / 10) /
      (MAX_INTERVAL_MS -
       ((data->game_info.level - 1) * (MAX_INTERVAL_MS - MIN_INTERVAL_MS) / 9));
  data->shift_interval =
      NS_IN_MS * (MAX_INTERVAL_MS - ((data->game_info.level - 1) *
                                     (MAX_INTERVAL_MS - MIN_INTERVAL_MS) / 9));
}

void get_random_figure(int** dst) {
  static bool initialized = false;
  static const int
      figures[FIGURES_COUNT][FIGURE_MAX_LENGTH][FIGURE_MAX_LENGTH] = {
          {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}},
          {{0, 0, 0, 0}, {1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}},
          {{0, 0, 0, 0}, {0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}},
          {{0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}},
          {{0, 0, 0, 0}, {0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}},
          {{0, 0, 0, 0}, {0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}},
          {{0, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}}};

  if (!initialized) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    srand((unsigned int)(ts.tv_nsec + (ts.tv_sec * NS_IN_S)));
    initialized = true;
  }

  int type = rand() % FIGURES_COUNT;
  for (int i = 0; i < FIGURE_MAX_LENGTH; i++) {
    for (int j = 0; j < FIGURE_MAX_LENGTH; j++) {
      dst[i][j] = figures[type][i][j];
    }
  }
}

void update_figure() {
  Game_t* data = staticGame();
  copy_matrix(data->game_info.next, data->current_figure.matrix,
              FIGURE_MAX_LENGTH, FIGURE_MAX_LENGTH);
  get_random_figure(data->game_info.next);
  data->current_figure.rotation = 0;
  data->current_figure.x = FIGURE_INIT_X;
  data->current_figure.y = FIGURE_INIT_Y;
  data->current_figure.init_place = true;
}

void place_current_figure() {
  Game_t* data = staticGame();
  merge_matrix(data->current_figure.matrix, data->game_info.field,
               FIGURE_MAX_LENGTH, FIGURE_MAX_LENGTH, data->current_figure.y,
               data->current_figure.x, PLACE);
}

void remove_placed_figure() {
  Game_t* data = staticGame();
  merge_matrix(data->current_figure.matrix, data->game_info.field,
               FIGURE_MAX_LENGTH, FIGURE_MAX_LENGTH, data->current_figure.y,
               data->current_figure.x, REMOVE);
}

void fall_figure() { staticGame()->current_figure.y += 1; }

void move_figure(int direction) { staticGame()->current_figure.x += direction; }

int shift_filled_lines() {
  return clear_filled_rows_matrix(staticGame()->game_info.field, FIELD_H,
                                  FIELD_W);
}

int check_overflow() {
  Game_t* data = staticGame();
  return merge_matrix(data->current_figure.matrix, data->game_info.field,
                      FIGURE_MAX_LENGTH, FIGURE_MAX_LENGTH,
                      data->current_figure.y, data->current_figure.x, CHECK);
}

int check_collision_down() {
  Game_t* data = staticGame();
  int result = 0;
  for (int i = FIGURE_MAX_LENGTH - 1; i >= 0; i--) {
    for (int j = 0; j < FIGURE_MAX_LENGTH; j++) {
      if (data->current_figure.matrix[i][j] != 0) {
        int pos_y_under = data->current_figure.y + i + 1;
        int pos_x = data->current_figure.x + j;
        if (pos_y_under >= FIELD_H || data->game_info.field[pos_y_under][pos_x])
          result = 1;
      }
    }
  }
  return result;
}

int check_collision_left() {
  Game_t* data = staticGame();
  int result = 0;
  for (int j = 0; j < FIGURE_MAX_LENGTH; j++) {
    for (int i = 0; i < FIGURE_MAX_LENGTH; i++) {
      if (data->current_figure.matrix[i][j] != 0) {
        int pos_x_left = data->current_figure.x + j - 1;
        int pos_y = data->current_figure.y + i;
        if (pos_x_left < 0 || data->game_info.field[pos_y][pos_x_left])
          result = 1;
      }
    }
  }
  return result;
}

int check_collision_right() {
  Game_t* data = staticGame();
  int result = 0;
  for (int j = FIGURE_MAX_LENGTH - 1; j >= 0; j--) {
    for (int i = 0; i < FIGURE_MAX_LENGTH; i++) {
      if (data->current_figure.matrix[i][j] != 0) {
        int pos_x_right = data->current_figure.x + j + 1;
        int pos_y = data->current_figure.y + i;
        if (pos_x_right >= FIELD_W || data->game_info.field[pos_y][pos_x_right])
          result = 1;
      }
    }
  }
  return result;
}