#include <stdint.h>
#include "defs.h"
#include "ext.h"

struct test 
{
  time_t *tm;
  time_t lastcheck;
};

struct test2
{
  uint32_t *tm;
  uint32_t lastcheck;
};
   

void main()
  
{
  printf("long: %i\n", sizeof(long));
  printf("short: %i\n", sizeof(short));
  printf("int: %i\n", sizeof(int));
  printf("time_t: %i\n", sizeof(time_t));
  printf("char: %i\n", sizeof(char));
  printf("pid_t: %i\n", sizeof(pid_t));
  printf("fd_set: %i\n", sizeof(fd_set));
   
  printf("int8_t: %i\n", sizeof(int8_t));
  printf("int: %i\n", sizeof(int));
  printf("int16_t: %i\n", sizeof(int16_t));
  printf("int32_t: %i\n", sizeof(int32_t));
  printf("uint32_t: %i\n", sizeof(uint32_t));
   
  printf("uint8_t * %i:\n", sizeof(uint8_t *));

  printf("bigbtmp: %i\n", sizeof(struct bigbtmp));
  //printf("btmp: %i\n", sizeof(struct btmp));
  //printf("qtmp: %i\n", sizeof(struct qtmp));
  printf("tm: %i\n", sizeof(struct tm *));

  //printf("bigbtmp without b and q: %i\n", sizeof(struct bigbtmp) -
  //sizeof(struct btmp)*MAXUSERS - sizeof(struct qtmp) * MAXQ);

  printf("inaddr: %i\n", sizeof(struct in_addr));
   
 }
   