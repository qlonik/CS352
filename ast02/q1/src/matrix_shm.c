#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <pthread.h>

#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

/* #include <time.h> */

#include "matrixOps.h"

#define DEBUG 0

#define MAX_BUF_LEN 1024
#define SHM_NAME "mShm"
#define SHM_SIZE 32 * 1024 // 8 * 4096
#define MAX_DELAY 10

int shm;
struct matrix m_global;
typedef struct {
   matrix m1, m2;
   int i, j;
} compute_matrix_cell; // parameter to threads

//print message to console and read number
int readNumber() {/*{{{*/
   int * digit = (int *) malloc( sizeof(int) );
   scanf("%d", digit);
   return * digit;
};/*}}}*/

int min(int x, int y) {/*{{{*/
   return (x < y) ? x : y;
}/*}}}*/

int max(int x, int y) {/*{{{*/
   return (x > y) ? x : y;
}/*}}}*/

void * thread( void * param ) {/*{{{*/
   compute_matrix_cell * arg = (compute_matrix_cell *) param;

   matrix m1 = arg->m1, m2 = arg->m2;
   int l, i = arg->i, j = arg->j;

   int k = min(*m1.c, *m2.r);
   m_global.m[i][j] = 0;

   for (l = 0; l < k; l++) {
      m_global.m[i][j] += m1.m[i][l] * m2.m[l][j];
      if (DEBUG) {
         printf("--- thread: pos:%d:l:%d:m1[i,l]:%d:m2[l,j]:%d:\n",
               i**m_global.c+j, l, m1.m[i][l], m2.m[l][j]);
      }
   }

   if (DEBUG) {
      printf("--- thread: pos:%d:i:%d:j:%d:m[i.j]:%d:\n",
            i * *m_global.c + j, i, j, m_global.m[i][j]);
   }

   pthread_exit(0);
}/*}}}*/

void safeExit() {/*{{{*/
   if (shm_unlink(SHM_NAME) == -1) {
      perror("error unlinking shm");
      exit(-1);
   }
   exit(0);
}/*}}}*/


int main() {
   int i, j, child_one, child_two;
   void * ptr, * startptr;


   child_one = fork();
   if ( child_one < 0 ) {
      perror("error creating first child");
      exit(1);
   } else if ( child_one == 0 ) {
      // child one code
      // this child read stdin and saves matrix to shm

      raise(SIGSTOP);/*{{{*/

      while (1) {
         printf("Enter matrix\n");
         printf("Enter number of rows and columns: ");
         int rows = readNumber(),
             cols = readNumber();

         matrix m = {
            .r = (int *) malloc( sizeof(int) ),
            .c = (int *) malloc( sizeof(int) ),
            .m = (int **) malloc( rows * sizeof(int ) )
         };
         *m.r = rows;
         *m.c = cols;
         for (i = 0; i < rows; i++) {
            m.m[i] = (int *) malloc( cols * sizeof(int) );
         }

         printf("Enter matrix data:\n");
         for (i = 0; i < rows; i++) {
            for (j = 0; j < cols; j++) {
               m.m[i][j] = readNumber();
            }
         }

         char * encodedMatrix = encodeMatrix(m);
         if (DEBUG) {
            printf("---- size of matrix struct:  %lu\n", sizeof(m));
            printf("---- size of encoded matrix: %lu\n", sizeof(encodedMatrix));
            printf("-- sent to shm: l:%lu:s:%s:\n", strlen(encodedMatrix), encodedMatrix);
         }


         shm = shm_open(SHM_NAME, O_RDWR, 0666);
         if (shm == -1) {
            perror("shm_open failed");
            exit(-1);
         }
         startptr = ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
         if (ptr == MAP_FAILED) {
            perror("map failed");
            exit(-1);
         }

         sprintf(ptr, "%s", encodedMatrix);
         ptr += strlen(encodedMatrix) + 1;

         raise(SIGSTOP);
      }/*}}}*/
   } else {
      child_two = fork();
      if ( child_two < 0 ) {
         perror("error creating second child");
         kill(child_one, SIGINT);
         exit(1);
      } else if ( child_two == 0 ) {
         // child two code

         raise(SIGSTOP);/*{{{*/

         while (1) {
            shm = shm_open(SHM_NAME, O_RDONLY, 0666);
            if (shm == -1) {
               perror("shm_open failed");
               exit(-1);
            }
            startptr = ptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm, 0);
            if (ptr == MAP_FAILED) {
               perror("map failed");
               exit(-1);
            }

            char * m_S = (char *) ptr;

            if (DEBUG) {
               printf("-- got from shm m: l:%lu:s:%s:\n", strlen(m_S), m_S);
            }

            //parse matrix
            struct matrix m = parseMatrix(m_S);

            //print matrix to stdout
            printf("Your matrix:\n");
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

         signal(SIGINT, safeExit);/*{{{*/
         signal(SIGALRM, safeExit);

         while (1) {
            shm = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
            ftruncate(shm, SHM_SIZE);
            startptr = ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
            if (ptr == MAP_FAILED) {
               perror("map failed");
               exit(-1);
            }

            alarm(MAX_DELAY);
            kill(child_one, SIGCONT);
            waitpid(child_one, NULL, WUNTRACED);

            char * m1_S = (char *) ptr;
            matrix m1 = parseMatrix(m1_S);

            alarm(MAX_DELAY);
            kill(child_one, SIGCONT);
            waitpid(child_one, NULL, WUNTRACED);

            char * m2_S = (char *) ptr;
            matrix m2 = parseMatrix(m2_S);

            if (DEBUG) {
               printf("-- first matrix from shm:  l:%lu:s:%s:\n", strlen(m1_S), m1_S);
               printf("-- second matrix from shm: l:%lu:s:%s:\n", strlen(m2_S), m2_S);
            }

            if (*(m1.c) != *(m2.r)) {
               perror("wrong input: columns in first matrix is not equal to rows in second matrix");
               exit(-1);
            }


            matrix m = {
               .r = (int *)malloc( sizeof(int) ),
               .c = (int *)malloc( sizeof(int) ),
               .m = (int **)malloc( *m1.r * sizeof(int *) )
            };
            *m.r = *m1.r;
            *m.c = *m2.c;
            for (i = 0; i < *m1.r; i++) {
               m.m[i] = (int *)malloc( *m2.c * sizeof(int) );
            }
            m_global = m;

            int num_of_threads = *(m.r) * *(m.c);
            pthread_t * tid = calloc(num_of_threads, sizeof(pthread_t));
            pthread_attr_t * tattr = calloc(num_of_threads, sizeof(pthread_attr_t));
            compute_matrix_cell * arg = calloc(num_of_threads, sizeof(compute_matrix_cell));

            for (i = 0; i < *(m_global.r); i++) {
               for (j = 0; j < *(m_global.c); j++) {
                  int pos = i * *(m_global.c) + j;
                  compute_matrix_cell a = arg[pos];

                  a.m1 = m1;
                  a.m2 = m2;
                  a.i = i;
                  a.j = j;

                  pthread_attr_init(&tattr[pos]);
                  pthread_create(&tid[pos], &tattr[pos], thread, &a);
                  pthread_join(tid[pos], NULL);
                  if (DEBUG) {
                     printf("--- pos:%d:tid:%lu: \n", pos, tid[pos]);
                  }
               }
            }

            char * m_S = encodeMatrix(m_global);
            if (DEBUG) {
               printf("-- l:%lu:s:%s:\n", strlen(m_S), m_S);
            }

            sprintf(ptr, "%s", m_S);
            ptr += strlen(m_S) + 1;

            kill(child_two, SIGCONT);
            waitpid(child_two, NULL, WUNTRACED);


            shm_unlink(SHM_NAME);

         }/*}}}*/
      }
   }

   return 0;
}




