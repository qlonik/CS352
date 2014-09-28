#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>

#include "matrixOps.h"

void printMessage(char msg[]) {
   if (msg != NULL) {
      printf("%s", msg);
   }
};

int readNumber(char msg[]) {
   printMessage(msg);

   int * digit = NULL;
   digit = malloc(0);
   scanf("%d", digit);
   return * digit;
};

int main() {
   // child_one - process reading from standard input
   // child_two - process reading from standard output
   int child_one, child_two;

   child_one = fork();
   if ( child_one < 0 ) {
      perror("error creating first child");
      exit(1);
   } else if ( child_one == 0 ) {
      // child one code
   } else {
      child_two = fork();
      if ( child_two < 0 ) {
         perror("error creating second child");
         exit(1);
      } else if (child_two == 0) {
         // child two code
      } else {
         // parent child
      }
   }

   return 0;
};
