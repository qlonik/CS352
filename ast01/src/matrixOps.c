#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "matrixOps.h"

struct matrix parseMatrix(char str[]) {/*{{{*/
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
   for (n = 0; n < strlen(copy); n++) {
      token = *(copy + n);

      if (token != SEMICOLON && token != COMMA) {
         l = strlen(buf);
         buf[l] = token;
         buf[l + 1] = '\0';
      } else {
         result.m[i][j] = strtol(buf, NULL, 10);

         buf[0] = 0;

         if (token == SEMICOLON) {
            i++;
            j = 0;
         }
         if (token == COMMA) {
            j++;
         }
      }
   }
   result.m[i][j] = strtol(buf, NULL, 10);

   free(buf);
   free(copy);

   return result;
};/*}}}*/
char * encodeMatrix(struct matrix matrix) {/*{{{*/
   int i, j, k, l;
   int * r = matrix.r, * c = matrix.c;
   int ** m = matrix.m;

   int els = *r * *c;
   int alloc_size = (MAX_STR_OF_INT_LEN * els + els - 1) * sizeof(char);

   char * result, * buf;
   result = (char *) malloc( alloc_size );
   buf = (char *) malloc( MAX_STR_OF_INT_LEN * sizeof(char) );

   for (i = 0; i < * r; i++) {
      for (j = 0; j < * c; j++) {
         sprintf(buf, "%d", m[i][j]);

         l = strlen(result);
         for (k = 0; k < strlen(buf); k++) {
            result[l + k] = buf[k];
         }
         result[l + k] = COMMA;
         result[l + k + 1] = '\0';
      }
      result[strlen(result) - 1] = ';';
   }
   result[strlen(result) - 1] = '\0';

   free(buf);

   return result;
};/*}}}*/


struct matrix multMatrix(struct matrix a, struct matrix b) {
   if (*a.c != *b.r) {
      //error
   }

   int i, j, k;
   int rows = *a.r, cols = *b.c;

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

   for (i = 0; i < *a.r; i++) {
      for (j = 0; j < *b.c; j++) {
         result.m[i][j] = 0;
         for (k = 0; k < *a.c; k++) {
            result.m[i][j] += a.m[i][k] * b.m[k][j];
         }
      }
   }

   return result;
};

