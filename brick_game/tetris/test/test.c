#include <check.h>
#include <stdlib.h>
#include <unistd.h>

#include "tetris.h"

#ifdef __linux__
#define SEED 674326212
#else
#define SEED 0
#endif

void print_field() {
  printf(" ----------\n");
  for (int i = 0; i < 20; i++) {
    for (int j = 0; j < 10; j++) {
      if (j == 0) printf("|");
      if (staticGame()->game_info.field[i][j])
        printf("%c", '#');
      else
        printf("%c", ' ');
      if (j == 9) printf("|\n");
    }
  }
  printf(" ----------\n");
}

void free_snapshot(GameInfo_t* snapshot) {
  if (snapshot->field) {
    for (int i = 0; i < FIELD_H; i++) free(snapshot->field[i]);
    free(snapshot->field);
  }
  if (snapshot->next) {
    for (int i = 0; i < FIGURE_MAX_LENGTH; i++) free(snapshot->next[i]);
    free(snapshot->next);
  }
}

void input(UserAction_t act, int n) {
  for (int i = 0; i < n; i++) {
    srand(SEED);
    userInput(act, false);
    GameInfo_t info = updateCurrentState();
    free_snapshot(&info);
  }
}

START_TEST(test_side_borders) {
  staticGame();
  srand(SEED);
  update_figure();

  input(Start, 1);
  input(Right, 6);
  input(Action, 4);
  ck_assert_int_eq(check_collision_right(), 1);
  input(Left, 5);
  ck_assert_int_eq(check_collision_left(), 1);
  free_game_memory();
}
END_TEST

START_TEST(test_1row) {
  staticGame();
  srand(SEED);
  update_figure();

  input(Start, 1);
  input(Left, 3);
  input(Down, 42);
  input(Right, 1);
  input(Down, 60);

  input(Right, 3);
  input(Action, 1);
  input(Down, 30);
  input(Action, 1);
  input(Right, 4);
  input(Down, 30);
  ck_assert(staticGame()->game_info.score == 100);
  free_game_memory();
}
END_TEST

START_TEST(test_2rows) {
  staticGame();
  srand(SEED);
  update_figure();

  input(Start, 1);
  input(Left, 3);
  input(Down, 42);
  input(Left, 3);
  input(Down, 42);
  input(Right, 1);
  input(Down, 40);
  input(Right, 1);
  input(Down, 60);

  input(Right, 3);
  input(Action, 1);
  input(Down, 30);
  input(Action, 1);
  input(Right, 4);
  input(Down, 30);
  ck_assert(staticGame()->game_info.score == 300);
  free_game_memory();
}
END_TEST

START_TEST(test_3rows) {
  staticGame();
  srand(SEED);
  update_figure();

  input(Start, 1);
  input(Left, 3);
  input(Down, 42);
  input(Left, 3);
  input(Down, 42);
  input(Left, 3);
  input(Down, 42);
  input(Right, 1);
  input(Down, 40);
  input(Right, 1);
  input(Down, 40);
  input(Right, 1);
  input(Down, 60);

  input(Right, 3);
  input(Action, 1);
  input(Down, 30);
  input(Action, 1);
  input(Right, 4);
  input(Down, 30);
  ck_assert(staticGame()->game_info.score == 700);
  free_game_memory();
}
END_TEST

START_TEST(test_4rows) {
  staticGame();
  srand(SEED);
  update_figure();

  input(Start, 1);
  input(Left, 3);
  input(Down, 42);
  input(Left, 3);
  input(Down, 42);
  input(Left, 3);
  input(Down, 42);
  input(Left, 3);
  input(Down, 42);
  input(Right, 1);
  input(Down, 40);
  input(Right, 1);
  input(Down, 40);
  input(Right, 1);
  input(Down, 40);
  input(Right, 1);
  input(Down, 50);

  input(Right, 3);
  input(Action, 1);
  input(Down, 30);
  input(Action, 1);
  input(Right, 4);
  input(Down, 25);
  ck_assert(staticGame()->game_info.score == 1500);
  free_game_memory();
}
END_TEST

START_TEST(test_10lvl) {
  staticGame();
  srand(SEED);
  update_figure();

  input(Start, 1);
  for (int i = 0; i < 5; i++) {
    input(Left, 3);
    input(Down, 42);
    input(Left, 3);
    input(Down, 42);
    input(Left, 3);
    input(Down, 42);
    input(Left, 3);
    input(Pause, 2);
    input(Down, 42);
    input(Right, 1);
    input(Down, 40);
    input(Right, 1);
    input(Down, 40);
    input(Right, 1);
    input(Down, 40);
    input(Right, 1);
    input(Down, 50);

    input(Right, 3);
    input(Action, 1);
    input(Down, 30);
    input(Action, 1);
    input(Right, 4);
    input(Down, 20);
  }
  ck_assert(staticGame()->game_info.level == 10 &&
            staticGame()->game_info.score > 6600);
  free_game_memory();
}
END_TEST

START_TEST(test_shift) {
  staticGame();
  srand(SEED);
  update_figure();

  input(Start, 1);
  input(Right, 1);
  usleep(1000000);
  input(Right, 1);
  usleep(1000000);
  input(Right, 1);
  usleep(1000000);
  input(Right, 1);
  ck_assert(!staticGame()->game_info.field[0][7] &&
            !staticGame()->game_info.field[1][7] &&
            staticGame()->game_info.field[2][7]);
  free_game_memory();
}
END_TEST

START_TEST(test_gameover) {
  staticGame();
  srand(SEED);
  update_figure();

  input(Start, 1);
  input(Down, 8);
  input(Action, 1);
  input(Down, 36);
  input(Action, 1);
  input(Down, 30);
  input(Action, 1);
  input(Down, 20);
  input(Action, 1);
  input(Down, 40);
  ck_assert(staticGame()->state == START);
  input(Terminate, 1);
  free_game_memory();
  staticGame()->terminated = false;
}
END_TEST

Suite* logic_tests() {
  Suite* s = suite_create("\033[45mGAME LOGIC TESTS\033[0m");
  TCase* tc = tcase_create("logic_tests");

  tcase_add_test(tc, test_side_borders);
  tcase_add_test(tc, test_1row);
  tcase_add_test(tc, test_2rows);
  tcase_add_test(tc, test_3rows);
  tcase_add_test(tc, test_4rows);
  tcase_add_test(tc, test_10lvl);
  tcase_add_test(tc, test_shift);
  tcase_add_test(tc, test_gameover);

  suite_add_tcase(s, tc);
  return s;
}

int run_suite(Suite* suite) {
  SRunner* suite_runner = srunner_create(suite);
  srunner_set_fork_status(suite_runner, CK_NOFORK);
  srunner_run_all(suite_runner, CK_NORMAL);
  int failed = srunner_ntests_failed(suite_runner);
  srunner_free(suite_runner);
  return failed;
}

int main() {
  int failed = 0;
  Suite* suites[] = {logic_tests(), NULL};

  for (int i = 0; suites[i] != NULL; i++) {
    failed += run_suite(suites[i]);
  }

  printf("========= FAILED: %d =========\n", failed);
  return failed != 0;
}
