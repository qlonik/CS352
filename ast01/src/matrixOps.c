#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "matrixOps.h"

/*
 * get string and output matrix representation of string
 */
struct matrix parseMatrix(char * str) {/*{{{*/
   char * copy = malloc(strlen(str) + 1);
   strcpy(copy, str);

   int i, j, n, l;
   //sc - semicolon count
   //cc - comma count
   //rows - total amount of rows in matrix
   //cols - total amount of columns in matrix
   int sc = 0, cc = 0, rows = 0, cols = 0;
   for (i = 0; i < strlen(copy); i++) {
      char token = *(copy + i);
      if (token == SEMICOLON) {
         sc++;
      }
      if (token == COMMA) {
         cc++;
      }
   }
   sc++;
   cc += sc;
   rows = sc;
   cols = cc / sc;

   /* printf("Total amount of rows: %d\n", sc); */
   /* printf("Total amount of elements: %d\n", cc); */
   /* printf("Amount of elements on each row: %d\n", cc / sc); */

   /* if (cc % sc != 0) { */
   /*    printf("Wrong matrix input\n"); */
   /*    return 1; */
   /* } */

   //create and assign matrix struct
   struct matrix result = {
      .r = (int *)malloc( sizeof(int) ),
      .c = (int *)malloc( sizeof(int) ),
      .m = (int **)malloc( rows * sizeof(int *) )
   };
   *result.r = rows;
   *result.c = cols;
   for (i = 0; i < rows; i++) {
      result.m[i] = (int *)malloc( cols * sizeof(int) );
   }

   char token;
   char * buf;
   buf = (char *)malloc(MAX_STR_OF_INT_LEN * sizeof(char));

   i = 0;
   j = 0;
   //start parsing passed string
   for (n = 0; n < strlen(copy); n++) {
      //get one character from string
      token = *(copy + n);

      if (token != SEMICOLON && token != COMMA) {
         //if token is not semicolon or comma, we save it into buffer
         //because we are currently reading the number
         l = strlen(buf);
         buf[l] = token;
         buf[l + 1] = '\0';
      } else {
         //if token is semicolon or comma, we convert number that we got to
         //int and save it to corresponding slot of matrix
         result.m[i][j] = strtol(buf, NULL, 10);

         //empty the buffer
         buf[0] = '\0';

         //update position for the next number based on given semicolon or
         //comma
         if (token == SEMICOLON) {
            i++;
            j = 0;
         }
         if (token == COMMA) {
            j++;
         }
      }
   }
   //save the last number, because it will not have neither semicolon nor comma
   //after it
   result.m[i][j] = strtol(buf, NULL, 10);

   //free memory
   free(buf);
   free(copy);

   return result;
};/*}}}*/
/*
 * get matrix and output string representation of the matrix
 */
char * encodeMatrix(struct matrix matrix) {/*{{{*/
   int i, j, k, l;
   //get values from matrix to convert to string
   int * r = matrix.r, * c = matrix.c;
   int ** m = matrix.m;

   //total amount of elements in matrix
   int els = *r * *c;
   //magically count necessary string length
   //(this number is exceeding what is actually required to be)
   int alloc_size = (MAX_STR_OF_INT_LEN * els + els - 1) * sizeof(char);

   //result string and temporary string buffer for a number
   char * result, * buf;
   result = (char *) malloc( alloc_size );
   buf = (char *) malloc( MAX_STR_OF_INT_LEN * sizeof(char) );

   for (i = 0; i < * r; i++) {
      for (j = 0; j < * c; j++) {
         //convert int to string
         sprintf(buf, "%d", m[i][j]);

         //copy buf into result string
         l = strlen(result);
         for (k = 0; k < strlen(buf); k++) {
            result[l + k] = buf[k];
         }
         //add delimeter and end of the string
         result[l + k] = COMMA;
         result[l + k + 1] = '\0';
      }
      //replace comma delimeter with semicolon delimeter where it is required
      result[strlen(result) - 1] = SEMICOLON;
   }
   result[strlen(result) - 1] = '\0';

   //free memory
   free(buf);

   return result;
};/*}}}*/


/*
 * mutliply 2 matrixes and output result matrix
 */
struct matrix multMatrix(struct matrix a, struct matrix b) {/*{{{*/
   if (*a.c != *b.r) {
      //error
   }

   int i, j, k;
   int rows = *a.r, cols = *b.c;

   //create and assign matrix struct
   struct matrix result = {
      .r = (int *)malloc( sizeof(int) ),
      .c = (int *)malloc( sizeof(int) ),
      .m = (int **)malloc( rows * sizeof(int *) )
   };
   *result.r = rows;
   *result.c = cols;
   for (i = 0; i < rows; i++) {
      result.m[i] = (int *)malloc( cols * sizeof(int) );
   }

   //multiply matrixes
   for (i = 0; i < *a.r; i++) {
      for (j = 0; j < *b.c; j++) {
         result.m[i][j] = 0;
         for (k = 0; k < *a.c; k++) {
            result.m[i][j] += a.m[i][k] * b.m[k][j];
         }
      }
   }

   return result;
};/*}}}*/

//testing current module
/*
int main(int argc, char *argv[]) {
   int i, j;
   struct matrix m;
   char * m_en_S;

   char m2x2_S[] = "1,2;3,4";
   char m3x3_S[] = "1,2,3;4,5,6;7,8,9";
   char m3x3_2_S[] = "9,8,7;6,5,4;3,2,1";
   char m4x4_S[] = "1,2,3,4;5,6,7,8;9,10,11,12;13,14,15,16";

   struct matrix m2x2 = parseMatrix(m2x2_S);
   struct matrix m3x3 = parseMatrix(m3x3_S);
   struct matrix m3x3_2 = parseMatrix(m3x3_2_S);
   struct matrix m4x4 = parseMatrix(m4x4_S);

   struct matrix m3x3_M = multMatrix(m3x3, m3x3_2);

   char * m2x2_en_S = encodeMatrix(m2x2);
   char * m3x3_en_S = encodeMatrix(m3x3);
   char * m3x3_2_en_S = encodeMatrix(m3x3_2);
   char * m3x3_M_en_S = encodeMatrix(m3x3_M);
   char * m4x4_en_S = encodeMatrix(m4x4);

   m = m2x2;
   m_en_S = m2x2_en_S;

   printf("Your encoded matrix:\n");
   printf("%s\n", m_en_S);

   printf("Your matrix:\n");
   for (i = 0; i < *m.r; i++) {
      for (j = 0; j < *m.c; j++) {
         printf("%3d ", m.m[i][j]);
      }
      printf("\n");
   }


   m = m3x3;
   printf("First matrix:\n");
   for (i = 0; i < *m.r; i++) {
      for (j = 0; j < *m.c; j++) {
         printf("%3d ", m.m[i][j]);
      }
      printf("\n");
   }
   m = m3x3_2;
   printf("Second matrix:\n");
   for (i = 0; i < *m.r; i++) {
      for (j = 0; j < *m.c; j++) {
         printf("%3d ", m.m[i][j]);
      }
      printf("\n");
   }
   m = m3x3_M;
   printf("Your multiplied matrix:\n");
   for (i = 0; i < *m.r; i++) {
      for (j = 0; j < *m.c; j++) {
         printf("%3d ", m.m[i][j]);
      }
      printf("\n");
   }

   return 0;
}
*/
