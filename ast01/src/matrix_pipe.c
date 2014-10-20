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

//write to the pipe
//taken from provided programs
void writePipe(char * s, int pipe) {
   write(pipe, s, strlen(s));
};

//read from the pipe
//taken from provided programs
int readPipe(char * s, int pipe) {
   int l = 0, i;

   while ( (i = read(pipe, s, 1)) > 0 ) {
      s++;
      l++;
   }
   s[l+1] = '\0';

   return l;
}

//print message to console unless it is NULL
void printMessage(char msg[]) {
   if (msg != NULL) {
      printf("%s", msg);
   }
};

//print message to console and read number
int readNumber(char msg[]) {
   printMessage(msg);

   int * digit = (int *) malloc( sizeof(int) );
   scanf("%d", digit);
   return * digit;
};

//exit program with success status
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
         //child one reads matrix from stdin and outputs it to pipe
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

         //encode matrix
         char * encodedMatrix = encodeMatrix(m);
         //open pipe and write data
         int woPipe = open(pipeName, O_WRONLY);
         writePipe(encodedMatrix, woPipe);
         close(woPipe);

         //stop itself
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
            //child two reads matrix from pipe and prints it to stdout
            //open pipe and wait for data
            int roPipe = open(pipeName, O_RDONLY);
            bufLen = readPipe(buf, roPipe);
            close(roPipe);

            //parse matrix
            struct matrix m = parseMatrix(buf);

            //print matrix to stdout
            printMessage("Your matrix:\n");
            for (i = 0; i < *m.r; i++) {
               for (j = 0; j < *m.c; j++) {
                  printf("%4d ", m.m[i][j]);
               }
               printf("\n");
            }

            //stop itself
            raise(SIGSTOP);
         }/*}}}*/
      } else {
         // parent code

         //initially stop children
         kill(child_one, SIGSTOP);
         kill(child_two, SIGSTOP);

         //respond to alarm by exiting program
         signal(SIGALRM, exitMatrix);

         while (1) {
            int rwPipe;
            char * m1_S = NULL, * m2_S = NULL;

            //start alarm
            alarm(MAX_DELAY);
            //continue first child
            kill(child_one, SIGCONT);
            //open pipe and wait for data
            rwPipe = open(pipeName, O_RDONLY);
            readPipe(buf, rwPipe);
            close(rwPipe);

            //copy read data from buf
            m1_S = malloc(strlen(buf));
            strcpy(m1_S, buf);

            //restart alarm
            alarm(MAX_DELAY);
            //continue first child again
            kill(child_one, SIGCONT);
            //open pipe and wait for data
            rwPipe = open(pipeName, O_RDONLY);
            readPipe(buf, rwPipe);
            close(rwPipe);

            //copy read data from buf
            m2_S = malloc(strlen(buf));
            strcpy(m2_S, buf);

            //parse inputted matrixes and multiply them
            struct matrix m1 = parseMatrix(m1_S),
                          m2 = parseMatrix(m2_S),
                          m = multMatrix(m1, m2);

            //encode multiplied matrix
            char * m_S = encodeMatrix(m);

            //continue second child
            kill(child_two, SIGCONT);
            //open pipe and write data into it
            rwPipe = open(pipeName, O_WRONLY);
            writePipe(m_S, rwPipe);
            close(rwPipe);

            //wait until second child finishes before starting new iteration
            waitpid(child_two, NULL, WUNTRACED);
         }
      }
   }

   return 0;
};
