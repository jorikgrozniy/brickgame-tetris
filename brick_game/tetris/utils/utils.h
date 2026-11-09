#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "brickgame.h"
#include "defines_tetris.h"
#include "tetris.h"

int** create_matrix(int rows, int cols);
void free_matrix(int** matrix, int rows);
void copy_matrix(int** src, int** dst, int rows, int cols);
int** deep_copy_matrix(int** src, int rows, int cols);
int merge_matrix(int** src, int** dst, int rows_src, int cols_src, int row_dst,
                 int col_dst, MatrixAction_t mode);
int clear_filled_rows_matrix(int** matrix, int rows, int cols);
void rotate_matrix(int** matrix, int size);

#endif