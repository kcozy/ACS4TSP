
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define BUFSIZE 1000

#define NUM 3293
#define NAME "japan3293"
#define COMMENT "prefectures, cities, towns and villages in Japan(2002.2.2)"
#define TYPE "TSP"
#define DIMENSION "3293"
#define EDGE_WEIGHT_TYPE "EUC_2D"

int main(int argc, char *argv[]) 
{
  char buf[BUFSIZE];
  char buf2[20];

  int tE, tN;
  int i;

  printf("NAME : ");
  printf(NAME);
  printf("\n");

  printf("COMMENT : ");
  printf(COMMENT);
  printf("\n");

  printf("TYPE : ");
  printf(TYPE);
  printf("\n");

  printf("DIMENSION : ");
  printf(DIMENSION);
  printf("\n");

  printf("EDGE_WEIGHT_TYPE : ");
  printf(EDGE_WEIGHT_TYPE);
  printf("\n");

  printf("NODE_COORD_SECTION\n");
    
  for(i = 0; i < NUM; i++) {
    if(fgets(buf, BUFSIZE, stdin) == NULL) {
      fprintf(stderr, "data error!\n");
      exit(1);
    }
    
    *(buf + 9) = '\0';
    *(buf + 20) = '\0';
    tE = atoi(buf) * 3600 + atoi(buf + 4) * 60 + atoi(buf + 7);
    tN = atoi(buf + 12) * 3600 + atoi(buf + 15) * 60 + atoi(buf + 18);

    

    printf("%d %d %d\n", i + 1, tE, tN);
  }
    
     
  return 0;
}
