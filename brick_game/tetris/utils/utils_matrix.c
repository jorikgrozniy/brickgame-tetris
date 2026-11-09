#include "utils.h"

int** create_matrix(int rows, int cols) {
  int** matrix = (int**)malloc(rows * sizeof(int*));
  for (int i = 0; i < rows; i++) {
    matrix[i] = (int*)calloc(cols, sizeof(int));
  }
  return matrix;
}

void free_matrix(int** matrix, int rows) {
  for (int i = 0; i < rows; i++) {
    free(matrix[i]);
  }
  free(matrix);
}

void copy_matrix(int** src, int** dst, int rows, int cols) {
  for (int i = 0; i < rows; i++) {
    memcpy(dst[i], src[i], cols * sizeof(int));
  }
}

int** deep_copy_matrix(int** src, int rows, int cols) {
  int** dst = (int**)malloc(rows * sizeof(int*));
  for (int i = 0; i < rows; i++) {
    dst[i] = malloc(cols * sizeof(int));
    memcpy(dst[i], src[i], cols * sizeof(int));
  }
  return dst;
}

int merge_matrix(int** src, int** dst, int rows_src, int cols_src, int row_dst,
                 int col_dst, MatrixAction_t mode) {
  int result = 0;
  for (int i = 0; i < rows_src; i++) {
    for (int j = 0; j < cols_src; j++) {
      int pos_x = col_dst + j;
      int pos_y = row_dst + i;
      if (src[i][j] != 0) {
        if (mode == PLACE)
          dst[pos_y][pos_x] = src[i][j];
        else if (mode == REMOVE)
          dst[pos_y][pos_x] = 0;
        else if (mode == CHECK &&
                 (pos_y >= FIELD_H || pos_y < 0 || pos_x >= FIELD_W ||
                  pos_x < 0 || src[i][j] == dst[pos_y][pos_x]))
          result = 1;
      }
    }
  }
  return result;
}

int clear_filled_rows_matrix(int** matrix, int rows, int cols) {
  int count = 0;
  for (int i = rows - 1; i >= 0; i--) {
    int row_filled = 1;
    for (int j = 0; j < cols; j++) {
      if (matrix[i][j] == 0) row_filled = 0;
    }

    if (row_filled) {
      free(matrix[i]);
      for (int k = i; k > 0; k--) {
        matrix[k] = matrix[k - 1];
      }
      matrix[0] = (int*)calloc(cols, sizeof(int));
      count++, i++;
    }
  }

  return count;
}

void rotate_matrix(int** matrix, int size) {
  for (int layer = 0; layer < size / 2; layer++) {
    int last = size - 1 - layer;
    for (int i = layer; i < last; i++) {
      int offset = i - layer;
      int top = matrix[layer][i];

      matrix[layer][i] = matrix[last - offset][layer];
      matrix[last - offset][layer] = matrix[last][last - offset];
      matrix[last][last - offset] = matrix[i][last];
      matrix[i][last] = top;
    }
  }
}