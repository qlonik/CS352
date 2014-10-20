#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>

#include <signal.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/socket.h>
#include <sys/un.h>

#include "vectorOps.h"

#define DEFAULT_PROTOCOL 0
#define MAX_BUF_LEN 1024
#define MAX_DELAY 10
#define DEBUG 0

void writeFD(char * s, int fd) {/*{{{*/
   write(fd, s, strlen(s) + 1);
};/*}}}*/

int readFD(char * s, int fd) {/*{{{*/
   int n, m = 0;

   do {
      n = read(fd, s, 1);
      m++;
      if (DEBUG) {
         printf("--r- read %d chars :%s: at addr %p\n", n, s, s);
         if (*s == 0) {
            printf("--r- s is zero\n");
         }
      }
   } while((n > 0) && ( (*s++) != 0 ));

   if (DEBUG) {
      printf("--r- m: %d strlen: %lu s: %s\n", m, strlen(s - m), s - m);
   }
   return m - 1;
}/*}}}*/

void printMessage(char msg[]) {/*{{{*/
   if (msg != NULL) {
      printf("%s", msg);
   }
};/*}}}*/

int readNumber(char msg[]) {/*{{{*/
   printMessage(msg);

   int * digit = NULL;
   digit = malloc(0);
   scanf("%d", digit);
   return * digit;
};/*}}}*/

void exitMatrix() {/*{{{*/
   if (DEBUG) {
      printf("-p- Exit by timeout\n");
   }
   exit(EXIT_SUCCESS);
}/*}}}*/

int main() {
   // child_one - process reading from standard input
   // child_two - process writing to standard output
   int child_one, child_two, bufLen;

   char * sockName = "mSock";
   char * buf = (char *) malloc(sizeof(char) * MAX_BUF_LEN);


   // ignore death of a child signals to prevent zombies
   // this will st errno to 10, whn child process finishes
   signal(SIGCHLD, SIG_IGN);

   unlink(sockName);


   child_one = fork();
   if ( child_one < 0 ) {
      perror("error creating first child");
      exit(1);
   } else if ( child_one == 0 ) {
      // child one code/*{{{*/


      int fd, sockAddrLen, status;
      struct sockaddr_un sockAddr;
      struct sockaddr * sockAddrPtr;

      sockAddr.sun_family = AF_UNIX;
      strcpy(sockAddr.sun_path, sockName);

      sockAddrPtr = (struct sockaddr *) &sockAddr;
      sockAddrLen = sizeof(sockAddr);

      while (1) {
         printMessage("Enter vector xyz coordinates: ");
         int x = readNumber(NULL),
             y = readNumber(NULL),
             z = readNumber(NULL);

         struct vector v = {
            .x = x,
            .y = y,
            .z = z
         };

         char * encodedVector = encodeVector(v);

         fd = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
         do {
            status = connect(fd, sockAddrPtr, sockAddrLen);
            if (status < 0) {
               sleep(1);
               if (DEBUG) {
                  printf("-1- Child one reconnecting .. (errno: %d status: %d)\n", errno, status);
               }
            }
         } while (status < 0);
         if (DEBUG) {
            printf("-1- Successful connection (child one). fd: %d\n", fd);
         }

         writeFD(encodedVector, fd);
         close(fd);

         raise(SIGSTOP);
      }/*}}}*/
   } else {
      child_two = fork();
      if ( child_two < 0 ) {
         perror("error creating second child");
         kill(child_one, SIGINT);
         exit(1);
      } else if (child_two == 0) {
         // child two code/*{{{*/


         int fd, sockAddrLen, status;
         struct sockaddr_un sockAddr;
         struct sockaddr * sockAddrPtr;

         sockAddr.sun_family = AF_UNIX;
         strcpy(sockAddr.sun_path, sockName);

         sockAddrPtr = (struct sockaddr *) &sockAddr;
         sockAddrLen = sizeof(sockAddr);

         while (1) {
            fd = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
            do {
               status = connect(fd, sockAddrPtr, sockAddrLen);
               if (status < 0) {
                  sleep(1);
                  if (DEBUG) {
                     printf("-2- Child two reconnecting .. (errno: %d status: %d)\n", errno, status);
                  }
               }
            } while (status < 0);
            if (DEBUG) {
               printf("-2- Successful connection (child two). fd: %d\n", fd);
            }

            bufLen = readFD(buf, fd);
            close(fd);

            printf("Your result: %s\n", buf);

            raise(SIGSTOP);
         }/*}}}*/
      } else {
         // parent code/*{{{*/
         kill(child_one, SIGSTOP);
         kill(child_two, SIGSTOP);

         signal(SIGALRM, exitMatrix);


         int fd, fd_client, sockAddrLen, errcode;
         unsigned int sockAddrLen_client;
         struct sockaddr_un sockAddr,
                            sockAddr_client;
         struct sockaddr * sockAddrPtr,
                         * sockAddrPtr_client;


         sockAddr.sun_family = AF_UNIX;
         strcpy(sockAddr.sun_path, sockName);

         sockAddrPtr = (struct sockaddr *) &sockAddr;
         sockAddrLen = sizeof(sockAddr);
         sockAddrPtr_client = (struct sockaddr *) &sockAddr_client;
         sockAddrLen_client = sizeof(sockAddr_client);
         /* sockAddrLen_client = sizeof(sockAddrPtr_client); */

         fd = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
         if (DEBUG) {
            printf("-p- Socket fd created: %d\n", fd);
         }

         errcode = bind(fd, sockAddrPtr, sockAddrLen);
         if (DEBUG) {
            printf("-p- bind errno: %d errcode: %d\n", errno, errcode);
         }

         errcode = listen(fd, 5);
         if (DEBUG) {
            printf("-p- listen errno: %d errcode: %d\n", errno, errcode);
         }


         while (1) {
            char * v1_S = NULL, * v2_S = NULL;

            // first vector
            alarm(MAX_DELAY);
            kill(child_one, SIGCONT);

            // wait for connection from client
            do {
               fd_client = accept(fd, sockAddrPtr_client, &sockAddrLen_client);
            } while(fd_client < 0);
            if (DEBUG) {
               printf("-p- Accepted child one with fd number: %d\n", fd_client);
            }

            // read from buffer
            readFD(buf, fd_client);
            // wait before child stops
            waitpid(child_one, NULL, WUNTRACED);

            // close child socket
            close(fd_client);

            v1_S = malloc(strlen(buf));
            strcpy(v1_S, buf);


            // second vector
            alarm(MAX_DELAY);
            kill(child_one, SIGCONT);

            // wait for connection from client
            do {
               fd_client = accept(fd, sockAddrPtr_client, &sockAddrLen_client);
            } while(fd_client < 0);
            if (DEBUG) {
               printf("-p- Accepted child one with fd number: %d\n", fd_client);
            }

            // read from buffer
            readFD(buf, fd_client);
            // wait before child stops
            waitpid(child_one, NULL, WUNTRACED);

            // close child socket
            close(fd_client);

            v2_S = malloc(strlen(buf));
            strcpy(v2_S, buf);


            // calculate dot product
            if (DEBUG) {
               printf("-p- Received vector strings: %s %s\n", v1_S, v2_S);
            }
            struct vector v1 = parseVector(v1_S),
                          v2 = parseVector(v2_S);

            int dotProd = dotProductVector(v1, v2);
            sprintf(buf, "%d", dotProd);


            // output
            kill(child_two, SIGCONT);

            // wait for connection from client
            do {
               fd_client = accept(fd, sockAddrPtr_client, &sockAddrLen_client);
            } while(fd_client < 0);
            if (DEBUG) {
               printf("-p- Accepted child two with fd number: %d\n", fd_client);
            }

            if (DEBUG) {
               printf("-p- Writing into socket: %s\n", buf);
            }
            // write computation result into socket
            writeFD(buf, fd_client);
            // wait until child stops
            waitpid(child_two, NULL, WUNTRACED);

            // close child socket
            close(fd_client);
         }/*}}}*/
      }
   }

   return 0;
};
