#define SEMICOLON ';'
#define COMMA ','
#define MAX_STR_OF_INT_LEN 10

struct matrix {
   int * r;
   int * c;
   int ** m;
};

struct matrix parseMatrix(char []);
char * encodeMatrix(struct matrix);


struct matrix multMatrix(struct matrix, struct matrix);
