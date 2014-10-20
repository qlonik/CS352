#define COMMA ','
#define MAX_STR_OF_INT_LEN 10

struct vector {
   int x;
   int y;
   int z;
};

struct vector parseVector(char *);
char * encodeVector(struct vector);


int dotProductVector(struct vector, struct vector);
