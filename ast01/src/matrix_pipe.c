#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include <signal.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "matrixOps.h"

#define MAX_BUF_LEN 1024
#define MAX_DELAY 10

void writePipe(char * s, int pipe) {
   write(pipe, s, strlen(s));
};

int readPipe(char * s, int pipe) {
   int l = 0, i;

   while ( (i = read(pipe, s, 1)) > 0 ) {
      s++;
      l++;
   }
   s[l+1] = '\0';

   return l;
}

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

void exitMatrix() {
   exit(EXIT_SUCCESS);
}

int main() {
   int i, j;
   // child_one - process reading from standard input
   // child_two - process reading from standard output
   int child_one, child_two, bufLen;

   char * pipeName = "mPipe";
   char * buf;
   buf = (char *) malloc(sizeof(char) * MAX_BUF_LEN);

   mkfifo(pipeName, S_IRWXU | S_IRWXO);

   child_one = fork();
   if ( child_one < 0 ) {
      perror("error creating first child");
      exit(1);
   } else if ( child_one == 0 ) {
      // child one code
      while (1) {/*{{{*/
         printMessage("Enter matrix\n");
         int rows = readNumber("Enter number of rows in matrix: "),
             cols = readNumber("Enter number of cols in matrix: ");

         struct matrix m = {
            .r = (int *) malloc( sizeof(int) ),
            .c = (int *) malloc( sizeof(int) ),
            .m = (int **) malloc( rows * sizeof(int *) )
         };
         *m.r = rows;
         *m.c = cols;
         for (i = 0; i < rows; i++) {
            m.m[i] = (int *) malloc( cols * sizeof(int) );
         }

         printMessage("Enter matrix data:\n");
         for (i = 0; i < rows; i++) {
            for (j = 0; j < cols; j++) {
               m.m[i][j] = readNumber(NULL);
            }
         }

         char * encodedMatrix = encodeMatrix(m);
         int woPipe = open(pipeName, O_WRONLY);
         writePipe(encodedMatrix, woPipe);
         close(woPipe);

         raise(SIGSTOP);
      }/*}}}*/
   } else {
      child_two = fork();
      if ( child_two < 0 ) {
         perror("error creating second child");
         kill(child_one, SIGINT);
         exit(1);
      } else if (child_two == 0) {
         // child two code
         while (1) {/*{{{*/
            int roPipe = open(pipeName, O_RDONLY);
            bufLen = readPipe(buf, roPipe);
            close(roPipe);

            struct matrix m = parseMatrix(buf);

            printMessage("Your matrix:\n");
            for (i = 0; i < *m.r; i++) {
               for (j = 0; j < *m.c; j++) {
                  printf("%4d ", m.m[i][j]);
               }
               printf("\n");
            }

            raise(SIGSTOP);
         }/*}}}*/
      } else {
         // parent code

         kill(child_one, SIGSTOP);
         kill(child_two, SIGSTOP);

         signal(SIGALRM, exitMatrix);

         while (1) {
            int rwPipe;
            char * m1_S = NULL, * m2_S = NULL;

            alarm(MAX_DELAY);
            kill(child_one, SIGCONT);
            rwPipe = open(pipeName, O_RDONLY);
            readPipe(buf, rwPipe);
            close(rwPipe);

            m1_S = malloc(strlen(buf));
            m1_S = buf;

            alarm(MAX_DELAY);
            kill(child_one, SIGCONT);
            rwPipe = open(pipeName, O_RDONLY);
            readPipe(buf, rwPipe);
            close(rwPipe);

            m2_S = malloc(strlen(buf));
            m2_S = buf;

            struct matrix m1 = parseMatrix(m1_S),
                          m2 = parseMatrix(m2_S),
                          m = multMatrix(m1, m2);

            char * m_S = encodeMatrix(m);

            kill(child_two, SIGCONT);
            rwPipe = open(pipeName, O_WRONLY);
            writePipe(m_S, rwPipe);
            close(rwPipe);

            waitpid(child_two, NULL, WUNTRACED);
         }
      }
   }

   return 0;
};
