#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <stdbool.h>

#include <pthread.h>

#ifndef ST
#define ST 0
#endif

#ifndef DEBUG
#define DEBUG 0
#endif

#ifndef SOLVER_DEBUG
#define SOLVER_DEBUG 0
#endif

#ifndef PRINT_EACH_STEP
#define PRINT_EACH_STEP 0
#endif

#define MAX_BUF_LEN 1024

typedef struct sudoku {
   int      size; // equal to alphabet length
   int      cell_size; // width of cell, square root of size
   char   * alphabet; // all differnet symbols
   int   ** puzzle;
   char *** suppliedPuzzle;
} Sudoku;

typedef struct {
   int r, c, v;
   Sudoku * s;
} thread_attr;
typedef struct {
   int valid;
} thread_return;

Sudoku s;

int indexOf(char * str, char * substr) {/*{{{*/
   int i;
   char * pos = strstr(str, substr);

   if (pos == NULL) {
      i = -1;
   } else {
      i = (int)(pos - str);
   }

   return i;
};/*}}}*/

void * rowCheck_thread( void * param ) {/*{{{*/
   Sudoku * s;
   int i, v, r, c, max_r, max_c;
   thread_return * ret = malloc(sizeof(thread_return));
   thread_attr * attr = (thread_attr *) param;

   s = attr->s;
   r = attr->r;
   c = attr->c;
   v = attr->v;
   max_r = max_c = s->size;

   for (i = 0; i < max_c; i++) {
      if (DEBUG) {
         printf("---- row r:%d:c:%d:P:%2d:v:%d:\n", r, i, s->puzzle[r][i], v);
      }
      if (i != c && s->puzzle[r][i] == v) {
         if (DEBUG) { printf("---- row invalid!\n"); }
         ret->valid = 0;
         return (void *) ret;
      }
   }

   ret->valid = 1;
   return (void *) ret;
};/*}}}*/
void * columnCheck_thread( void * param ) {/*{{{*/
   Sudoku * s;
   int i, v, r, c, max_r, max_c;
   thread_return * ret = malloc(sizeof(thread_return));
   thread_attr * attr = (thread_attr *) param;

   s = attr->s;
   r = attr->r;
   c = attr->c;
   v = attr->v;
   max_r = max_c = s->size;

   for (i = 0; i < max_r; i++) {
      if (DEBUG) {
         printf("---- column r:%d:c:%d:P:%2d:v:%d:\n", i, c, s->puzzle[i][c], v);
      }
      if (i != r && s->puzzle[i][c] == v) {
         if (DEBUG) { printf("---- column invalid!\n"); }
         ret->valid = 0;
         return (void *) ret;
      }
   }

   ret->valid = 1;
   return (void *) ret;
};/*}}}*/
void * cellCheck_thread( void * param ) {/*{{{*/
   Sudoku * s;
   int i, j, v, r, c, sRow, sCol, max_cell;
   thread_return * ret = malloc(sizeof(thread_return));
   thread_attr * attr = (thread_attr *) param;

   s = attr->s;
   r = attr->r;
   c = attr->c;
   v = attr->v;
   max_cell = s->cell_size;
   sRow = (r / max_cell) * max_cell;
   sCol = (c / max_cell) * max_cell;

   for (i = sRow; i < sRow + max_cell; i++) {
      for (j = sCol; j < sCol + max_cell; j++) {
         if (DEBUG) {
            printf("---- cell r:%d:c:%d:P:%2d:v:%d:\n", i, j, s->puzzle[i][j], v);
         }
         if (i != r && j != c && s->puzzle[i][j] == v) {
            if (DEBUG) { printf("---- cell invalid!\n"); }
            ret->valid = 0;
            return (void *) ret;
         }
      }
   }

   ret->valid = 1;
   return (void *) ret;
};/*}}}*/
int checkValid(Sudoku * s, int r, int c, int v) {/*{{{*/
   int valid;
   pthread_t row_tid, column_tid, cell_tid;
   pthread_attr_t row_tattr, column_tattr, cell_tattr;
   thread_attr arg;
   thread_return * row_valid = malloc(sizeof(thread_return)),
                 * column_valid = malloc(sizeof(thread_return)),
                 * cell_valid = malloc(sizeof(thread_return));

   arg.s = s;
   arg.r = r;
   arg.c = c;
   arg.v = v;

   pthread_attr_init(&row_tattr);
   pthread_attr_init(&column_tattr);
   pthread_attr_init(&cell_tattr);

   pthread_create(&row_tid, &row_tattr, rowCheck_thread, &arg);
   if (DEBUG) { printf("--- row    tid:%lu:\n", row_tid); }
   pthread_create(&column_tid, &column_tattr, columnCheck_thread, &arg);
   if (DEBUG) { printf("--- column tid:%lu:\n", column_tid); }
   pthread_create(&cell_tid, &cell_tattr, cellCheck_thread, &arg);
   if (DEBUG) { printf("--- cell   tid:%lu:\n", cell_tid); }

   pthread_join(row_tid, (void *) &row_valid);
   pthread_join(column_tid, (void *) &column_valid);
   pthread_join(cell_tid, (void *) &cell_valid);

   if (DEBUG) {
      printf("--- row    valid:%d:\n", row_valid->valid);
      printf("--- column valid:%d:\n", column_valid->valid);
      printf("--- cell   valid:%d:\n", cell_valid->valid);
   }


   if (row_valid->valid && column_valid->valid && cell_valid->valid) {
      valid = 1;
   } else {
      valid = 0;
   }

   free(row_valid);
   free(column_valid);
   free(cell_valid);

   return valid;
}/*}}}*/
int checkValid_sync(Sudoku * s, int r, int c, int v) {/*{{{*/
   int i, j, sRow, sCol, maxCellSize;
   for (i = 0; i < s->size; i++) {
      if (s->puzzle[i][c] == v || s->puzzle[r][i] == v)
         return 0;
   }

   maxCellSize = s->cell_size;

   sRow = (r / maxCellSize) * maxCellSize;
   sCol = (c / maxCellSize) * maxCellSize;

   for (i = sRow; i < sRow + maxCellSize; i++) {
      for (j = sCol; j < sCol + maxCellSize; j++) {
         if (s->puzzle[i][j] == v)
            return 0;
      }
   }

   return 1;
}/*}}}*/

Sudoku readSudoku(FILE * f) {/*{{{*/
   int i, j, size, cell_size;
   char * alph  = (char *) malloc(MAX_BUF_LEN * sizeof(char)),
        * buf = (char *) malloc(MAX_BUF_LEN * sizeof(char));

   fscanf(f, "%s", alph);
   size = strlen(alph);
   cell_size = (int) sqrt(size);

   if (DEBUG) {
      printf("--- alph:%s:size:%d:cell:%d\n", alph, size, cell_size);
   }

   // define sudoku
   Sudoku s = {
      .size = size,
      .cell_size = cell_size,
      .alphabet = (char *) malloc(MAX_BUF_LEN * sizeof(char)),
      .puzzle = (int **) malloc( size * sizeof(int *) ),
      .suppliedPuzzle = (char ***) malloc( size * sizeof(char **) )
   };
   strcpy(s.alphabet, alph);
   for (i = 0; i < size; i++) {
      s.puzzle[i] = (int *) malloc( size * sizeof(int) );
      s.suppliedPuzzle[i] = (char **) malloc( size * sizeof(char *) );
      for (j = 0; j < size; j++) {
         s.suppliedPuzzle[i][j] = (char *) malloc( MAX_BUF_LEN * sizeof(char) );
      }
   }

   for (i = 0; i < size; i++) {
      for (j = 0; j < size; j++) {
         fscanf(f, "%s", buf);
         strcpy(s.suppliedPuzzle[i][j], buf);

         if (DEBUG) {
            printf("---- buf:%s:i:%2d:\n", buf, indexOf(alph, buf));
         }

         if (*buf != '.') {
            s.puzzle[i][j] = indexOf(alph, buf);
         } else {
            s.puzzle[i][j] = -1;
         }
      }
   }

   return s;
};/*}}}*/
void printSudoku(Sudoku s) {/*{{{*/
   int i, j, k;
   char c;

   printf("Given sudoku\n");
   for (i = 0; i < s.size; i++) {
      for (j = 0; j < s.size; j++) {
         printf("%s ", s.suppliedPuzzle[i][j]);
      }
      printf("\n");
   }

   printf("Solved sudoku\n");
   for (i = 0; i < s.size; i++) {
      for (j = 0; j < s.size; j++) {
         k = s.puzzle[i][j];
         c = ((k == -1) ? '.' : s.alphabet[k]);

         printf("%c ", c);
      }
      printf("\n");
   }
};/*}}}*/
void debugPrintSudoku(Sudoku s) {/*{{{*/
   int i, j;
   char * buf = malloc(MAX_BUF_LEN * sizeof(char)),
        * d = malloc(MAX_BUF_LEN * sizeof(char)),
        * delim = malloc(MAX_BUF_LEN * sizeof(char));

   strcpy(buf, "");
   strcpy(delim, "--------------------------\n");

   for (i = 0; i < s.size; i++) {
      for (j = 0; j < s.size; j++) {
         sprintf(d, "%2d", s.puzzle[i][j]);
         strcat(buf, d);
         strcat(buf, " ");
      }
      strcat(buf, "\n");
   }

   strcat(buf, delim);
   printf("%s", buf);

   free(buf);
   free(d);
   free(delim);
}/*}}}*/
int solveSudoku(Sudoku * s, int r, int c) {/*{{{*/
   int v, valid;

   if (r == s->size) {
      return 1;
   }
   if (c == s->size) {
      return solveSudoku(s, r + 1, 0);
   }

   if (SOLVER_DEBUG) {
      printf("-- dbg; r:%d:c:%d:P:%d:\n", r, c, s->puzzle[r][c]);
   }

   if (s->puzzle[r][c] != -1) {
      return solveSudoku(s, r, c + 1);
   }

   for (v = 0; v < s->size; v++) {
      if (SOLVER_DEBUG) {
         printf("-- dbg; r:%d:c:%d:P:%d:v:%d:\n", r, c, s->puzzle[r][c], v);
      }
      if (ST) {
         valid = checkValid_sync(s, r, c, v);
      } else {
         valid = checkValid(s, r, c, v);
      }
      if (valid) {
         s->puzzle[r][c] = v;
         if (PRINT_EACH_STEP) {
            debugPrintSudoku(*s);
         }

         if (solveSudoku(s, r, c + 1)) {
            return 1;
         }
      }
   }

   s->puzzle[r][c] = -1;

   return 0;
}/*}}}*/

int main(int argc, char * argv[]) {
   if (argc > 2 || argc <= 1) {
      printf("Usage: %s <filename>\n", argv[0]);
      exit(1);
   }

   FILE * file = fopen(argv[1], "r");
   if (!file) {
      return 1;
   }
   if (DEBUG) {
      printf("--- file - p:%p:\n", file);
   }

   while (!feof(file)) {
      s = readSudoku(file);

      if (s.size > 0) {
         int result = solveSudoku(&s, 0, 0);
         printf("-------------------------------------------------\n");
         if (result) {
            printSudoku(s);
         } else {
            printf("No solution\n");
         }
      }
   }

   return 0;
};

