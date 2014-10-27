#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifndef DEBUG
#define DEBUG 0
#endif

#define nil 0

typedef struct Node {
   int info;
   struct Node * link;
} node;

typedef struct Queue {
   struct Node * front;
   struct Node * back;
} queue;

void initQueue(queue * q) {/*{{{*/
   q->front = nil;
   q->back = nil;
}/*}}}*/
void enqueue(queue * q, int info) {/*{{{*/
   node * n = (node *) malloc(sizeof(node));

   n->link = nil;
   n->info = info;

   if (q->front == nil) {
      q->front = n;
   } else {
      q->back->link = n;
   }
   q->back = n;
}/*}}}*/
void dequeue(queue * q, int * rinfo) {/*{{{*/
   node * n = q->front;

   if (q->front != nil) {
      *rinfo = q->front->info;
      q->front = q->front->link;
      free(n);
   }
}/*}}}*/

void debugPrintQueue(queue q) {/*{{{*/
   node * n = q.front;
   printf("f -> ");
   while (n != nil) {
      printf("%d -> ", n->info);
      n = n->link;
   }
   printf("b\n");
}/*}}}*/

int main(int argc, char * argv[]) {
   int n = 0, i, currPid, nextToResume = 0, totalOdd = 0, totalEven = 0,
       * pid, * wait;

   if (argc == 2) {
      n = strtol(argv[1], NULL, 10);
   } else {
      printf("Wrong input. Input format: %s <number>\n", argv[0]);
      exit(1);
   }

   if (DEBUG) {
      printf("- n: %d\n", n);
   }

   wait = (int *) calloc(n, sizeof(int));
   pid = (int *) calloc(n, sizeof(int));

   srand(time(NULL));
   for (i = 0; i < n; ++i) {
      wait[i] = rand() % 10;
      if (DEBUG) {
         printf("- r: %d\n", wait[i]);
      }
   }

   queue odd_pid, even_pid;
   initQueue(&odd_pid);
   initQueue(&even_pid);

   for (i = 0; i < n; i++) {
      currPid = fork();
      if (currPid == 0) {
         // child process
         break;
      } else {
         // parent

         if (DEBUG) {
            printf("--- child: i:%d:pid:%d:r:%d:\n", i, currPid, wait[i]);
         }

         kill(currPid, SIGSTOP);
         pid[i] = currPid;
         if (currPid % 2 == 1) {
            enqueue(&odd_pid, i);
         } else {
            enqueue(&even_pid, i);
         }
      }
   }

   if (currPid == 0) {
      // child process

      sleep(wait[i]);
      exit(0);
   } else {
      if (DEBUG) {
         debugPrintQueue(odd_pid);
         debugPrintQueue(even_pid);
      }

      while (odd_pid.front != nil || even_pid.front != nil) {
         if (odd_pid.front != nil) {
            dequeue(&odd_pid, &nextToResume);

            totalOdd += wait[nextToResume];
            printf("Child with pid %d is executing for %d seconds\n",
                  pid[nextToResume], wait[nextToResume]);

            kill(pid[nextToResume], SIGCONT);
            waitpid(pid[nextToResume], NULL, 0);
         }

         if (even_pid.front != nil) {
            dequeue(&even_pid, &nextToResume);

            totalEven += wait[nextToResume];
            printf("Child with pid %d is executing for %d seconds\n",
                  pid[nextToResume], wait[nextToResume]);

            kill(pid[nextToResume], SIGCONT);
            waitpid(pid[nextToResume], NULL, 0);
         }
      }

      printf("Total time assigned to odd queue:  %d\n", totalOdd);
      printf("Total time assigned to even queue: %d\n", totalEven);
      printf("Total time assigned for children:  %d\n", totalOdd + totalEven);
   }

   return 0;
}
