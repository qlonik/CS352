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

// sudoku structure
typedef struct sudoku {
   int      size; // equal to alphabet length
   int      cell_size; // width of cell, square root of size
   char   * alphabet; // all differnet symbols
   int   ** puzzle;
   char *** suppliedPuzzle;
} Sudoku;

// attribute to thread
typedef struct {
   int r, c, v;
   Sudoku * s;
} thread_attr;
// return from thread
typedef struct {
   int valid;
} thread_return;

// shared sudoku
Sudoku s;

// function returns index of character in the string
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

// thread fn to check if given value in the cell is correct for the row`
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
// thread fn to check if given value in the cell is correct for the column
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
// thread fn to check if given value in the cell is correct for the bigger cell
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
// fn to check if value is correct in the given cell
// implemented with thread fns above
int checkValid(Sudoku * s, int r, int c, int v) {/*{{{*/
   int valid;
   pthread_t row_tid, column_tid, cell_tid;
   pthread_attr_t row_tattr, column_tattr, cell_tattr;
   thread_attr arg;
   thread_return * row_valid = malloc(sizeof(thread_return)),
                 * column_valid = malloc(sizeof(thread_return)),
                 * cell_valid = malloc(sizeof(thread_return));

   // create argument object for each thread
   arg.s = s;
   arg.r = r;
   arg.c = c;
   arg.v = v;

   // prepare argument
   pthread_attr_init(&row_tattr);
   pthread_attr_init(&column_tattr);
   pthread_attr_init(&cell_tattr);

   // start thread that checks if row is correct
   pthread_create(&row_tid, &row_tattr, rowCheck_thread, &arg);
   if (DEBUG) { printf("--- row    tid:%lu:\n", row_tid); }
   // start thread that checks if column is correct
   pthread_create(&column_tid, &column_tattr, columnCheck_thread, &arg);
   if (DEBUG) { printf("--- column tid:%lu:\n", column_tid); }
   // start thread that checks if bigger cell is correct
   pthread_create(&cell_tid, &cell_tattr, cellCheck_thread, &arg);
   if (DEBUG) { printf("--- cell   tid:%lu:\n", cell_tid); }

   // wait until all three threads finish
   pthread_join(row_tid, (void *) &row_valid);
   pthread_join(column_tid, (void *) &column_valid);
   pthread_join(cell_tid, (void *) &cell_valid);

   if (DEBUG) {
      printf("--- row    valid:%d:\n", row_valid->valid);
      printf("--- column valid:%d:\n", column_valid->valid);
      printf("--- cell   valid:%d:\n", cell_valid->valid);
   }


   // if all three threads return valid for current combination of row, column
   // and value, then this value is possible for this cell
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
// fn to check if value is correct in the cell without threads
int checkValid_sync(Sudoku * s, int r, int c, int v) {/*{{{*/
   int i, j, sRow, sCol, maxCellSize;

   // check if row and column are correct
   for (i = 0; i < s->size; i++) {
      if (s->puzzle[i][c] == v || s->puzzle[r][i] == v)
         return 0;
   }

   maxCellSize = s->cell_size;

   sRow = (r / maxCellSize) * maxCellSize;
   sCol = (c / maxCellSize) * maxCellSize;

   // check if current bigger cell is correct
   for (i = sRow; i < sRow + maxCellSize; i++) {
      for (j = sCol; j < sCol + maxCellSize; j++) {
         if (s->puzzle[i][j] == v)
            return 0;
      }
   }

   return 1;
}/*}}}*/

// read one sudoku from file
Sudoku readSudoku(FILE * f) {/*{{{*/
   int i, j, size, cell_size;
   char * alph  = (char *) malloc(MAX_BUF_LEN * sizeof(char)),
        * buf = (char *) malloc(MAX_BUF_LEN * sizeof(char));

   // first line of the sudoku block is the alphabet
   fscanf(f, "%s", alph);
   // size of our sudoku
   size = strlen(alph);
   // size of our bigger cell
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
   // save alphabet
   strcpy(s.alphabet, alph);
   // allocate memory for suppliedPuzzle and puzzle
   for (i = 0; i < size; i++) {
      s.puzzle[i] = (int *) malloc( size * sizeof(int) );
      s.suppliedPuzzle[i] = (char **) malloc( size * sizeof(char *) );
      for (j = 0; j < size; j++) {
         s.suppliedPuzzle[i][j] = (char *) malloc( MAX_BUF_LEN * sizeof(char) );
      }
   }

   // read size*size symbols from the file and save them to suppliedPuzzle
   // section and convert them to numbers and save to puzzle section according
   // to the alphabet provided in the beginning
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
// print provided sudoku and solution
void printSudoku(Sudoku s) {/*{{{*/
   int i, j, k;
   char c;

   // print sudoku that was given
   printf("Given sudoku\n");
   for (i = 0; i < s.size; i++) {
      for (j = 0; j < s.size; j++) {
         printf("%s ", s.suppliedPuzzle[i][j]);
      }
      printf("\n");
   }

   // print solved sudoku using alphabet provided
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
// print internal representation of sudoku for debugging
void debugPrintSudoku(Sudoku s) {/*{{{*/
   int i, j;
   char * buf = malloc(MAX_BUF_LEN * sizeof(char)),
        * d = malloc(MAX_BUF_LEN * sizeof(char)),
        * delim = malloc(MAX_BUF_LEN * sizeof(char));

   // start buffer string
   strcpy(buf, "");
   // provide delimeter after printed sudoku
   strcpy(delim, "--------------------------\n");

   // build the buffer string to print to console
   for (i = 0; i < s.size; i++) {
      for (j = 0; j < s.size; j++) {
         sprintf(d, "%2d", s.puzzle[i][j]);
         strcat(buf, d);
         strcat(buf, " ");
      }
      strcat(buf, "\n");
   }

   // add delimeter
   strcat(buf, delim);
   // print to console
   printf("%s", buf);

   // free memory
   free(buf);
   free(d);
   free(delim);
}/*}}}*/
// recursive fn to solve sudoku, invoked with sudoku and starting cell 0,0
int solveSudoku(Sudoku * s, int r, int c) {/*{{{*/
   int v, valid;

   // if our current row is more than the size of the sudoku then we went
   // through all the elements of sudoku and we inserted all correct values
   // and the sudoku is solved
   if (r == s->size) {
      return 1;
   }
   // if our current column is more than the size of the sudoku then we went
   // throught the whole current row and we need to start checking next row;
   if (c == s->size) {
      return solveSudoku(s, r + 1, 0);
   }

   if (SOLVER_DEBUG) {
      // print debug info about current row, column and value in the current
      // cell
      printf("-- dbg; r:%d:c:%d:P:%d:\n", r, c, s->puzzle[r][c]);
   }

   // if value in current cell is not -1 we skip this cell
   // it means that if this value have already been provided, we skip the cell
   if (s->puzzle[r][c] != -1) {
      return solveSudoku(s, r, c + 1);
   }

   for (v = 0; v < s->size; v++) {
      if (SOLVER_DEBUG) {
         // print debug info about current row, column, cell value and value
         // that is being tested. Here value in the cell has always be -1
         printf("-- dbg; r:%d:c:%d:P:%d:v:%d:\n", r, c, s->puzzle[r][c], v);
      }
      // check if we can put the value v in the current cell. If macros set to
      // use one thread, then we use function without threads otherwise we use
      // function with threads.
      if (ST) {
         valid = checkValid_sync(s, r, c, v);
      } else {
         valid = checkValid(s, r, c, v);
      }
      // if we are able to insert this number to the cell then insert it and go
      // to the next cell. Otherwise, do not insert and go to next value of v.
      if (valid) {
         s->puzzle[r][c] = v;
         if (PRINT_EACH_STEP) {
            debugPrintSudoku(*s);
         }

         // go to the next cell after we inserted the value to the current cell
         if (solveSudoku(s, r, c + 1)) {
            return 1;
         }
      }
   }

   // we went through all the values that can be written in the current cell
   // and didnt find any matching one, then we need to return current cell
   // to the unknown value
   s->puzzle[r][c] = -1;

   return 0;
}/*}}}*/

int main(int argc, char * argv[]) {
   // filename has to be second argument
   if (argc > 2 || argc <= 1) {
      printf("Usage: %s <filename>\n", argv[0]);
      exit(1);
   }

   // open file for read
   FILE * file = fopen(argv[1], "r");
   if (!file) {
      return 1;
   }
   if (DEBUG) {
      // print file memory address
      printf("--- file - p:%p:\n", file);
   }

   // while there is stuff in the file
   while (!feof(file)) {
      // read sudoku
      s = readSudoku(file);

      // if this sudoku is not empty
      if (s.size > 0) {
         // solve sudoku and return result
         // 1 - solved
         // 0 - not solved
         int result = solveSudoku(&s, 0, 0);
         printf("-------------------------------------------------\n");
         if (result) {
            // print sudoku
            printSudoku(s);
         } else {
            printf("No solution\n");
         }
      }
   }

   return 0;
};

