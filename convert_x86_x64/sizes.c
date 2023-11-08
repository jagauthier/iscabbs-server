#include <stdint.h>
#include "defs.h"
  

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
  printf("int32_t: %i\n\n", sizeof(int32_t));
  printf("unsigned int: %i\n\n", sizeof(unsigned int));

  printf("bigbtmp: %i\n", sizeof(struct bigbtmp));
  
  printf("udata: %i\n", sizeof(struct userdata ));
  printf("user: %i\n", sizeof(struct user));

  printf("mheader: %i\n", sizeof(struct mheader));
  printf("all users: %i\n", sizeof(struct user)*MAXTOTALUSERS);


   
 }
   