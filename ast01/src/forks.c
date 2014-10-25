#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

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
   int n = 0, r, i;

   if (argc == 2) {
      n = strtol(argv[1], NULL, 10);
   } else {
      printf("Wrong input. Input format: %s <number>\n", argv[0]);
      exit(1);
   }

   printf("- n: %d\n", n);

   srand(time(NULL));
   for (i = 0; i < n; ++i) {
      r = rand() % 10;
      printf("- r: %d\n", r);
   }

   queue odd_pid, even_pid;
   initQueue(&odd_pid);
   initQueue(&even_pid);

   for (i = 0; i < n; i++) {
   }

   return 0;
}
