#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "vectorOps.h"

struct vector parseVector(char * str) {/*{{{*/
   char * copy = malloc(strlen(str) + 1);
   strcpy(copy, str);

   int i = 0, j = 0, l = 0;
   struct vector v = {
      .x = 0,
      .y = 0,
      .z = 0
   };

   char token;
   char * buf;
   buf = (char *) malloc(MAX_STR_OF_INT_LEN * sizeof(char));

   for (i = 0; i < strlen(copy); i++) {
      token = *(copy + i);

      if (token != COMMA) {
         l = strlen(buf);
         buf[l] = token;
         buf[l + 1] = '\0';
      } else {
         if (j == 0) {
            v.x = strtol(buf, NULL, 10);
         } else if (j == 1) {
            v.y = strtol(buf, NULL, 10);
         }

         buf[0] = '\0';
         j++;
      }
   }
   //save the last number
   if (j == 2) {
      v.z = strtol(buf, NULL, 10);
   }

   free(buf);
   free(copy);

   return v;
};/*}}}*/
char * encodeVector(struct vector v) {/*{{{*/
   int i, l = 0;
   char * result, * buf_x, * buf_y, * buf_z;
   buf_x = (char *) malloc( MAX_STR_OF_INT_LEN * sizeof(char) );
   buf_y = (char *) malloc( MAX_STR_OF_INT_LEN * sizeof(char) );
   buf_z = (char *) malloc( MAX_STR_OF_INT_LEN * sizeof(char) );

   sprintf(buf_x, "%d", v.x);
   sprintf(buf_y, "%d", v.y);
   sprintf(buf_z, "%d", v.z);

   l = strlen(buf_x) + strlen(buf_y) + strlen(buf_z) + 2;
   result = (char *) malloc(l);

   l = strlen(result);
   for ( i = 0; i < strlen(buf_x); i++ ) {
      result[l + i] = buf_x[i];
   }
   result[l + i] = COMMA;
   result[l + i + 1] = '\0';

   l = strlen(result);
   for ( i = 0; i < strlen(buf_y); i++ ) {
      result[l + i] = buf_y[i];
   }
   result[l + i] = COMMA;
   result[l + i + 1] = '\0';

   l = strlen(result);
   for ( i = 0; i < strlen(buf_z); i++ ) {
      result[l + i] = buf_z[i];
   }
   result[l + i] = '\0';

   free(buf_x);
   free(buf_y);
   free(buf_z);

   return result;
}/*}}}*/


int dotProductVector(struct vector a, struct vector b) {/*{{{*/
   int result = 0;

   result = a.x * b.x + a.y * b.y + a.z * b.z;

   return result;
};/*}}}*/
