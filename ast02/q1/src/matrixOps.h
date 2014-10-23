#define SEMICOLON ';'
#define COMMA ','
#define MAX_STR_OF_INT_LEN 10

/*
 * Matrix structure
 * r - row
 * c - column
 * m - 2d array with data
 */
typedef struct matrix {
   int * r;
   int * c;
   int ** m;
} matrix;

/*
 * get string and output matrix representation of string
 */
matrix parseMatrix(char *);
/*
 * get matrix and output string representation of the matrix
 */
char * encodeMatrix(matrix);


/*
 * mutliply 2 matrixes and output result matrix
 */
matrix multMatrix(matrix, matrix);
