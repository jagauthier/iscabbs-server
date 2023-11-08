#include <stdint.h>
#include <stdbool.h>
#include "defs.h"
#include "ext.h"

void monitor_bigbtmp(void) {
  uint8_t i;
  struct user *user = finduser("Dredd", 0, 0);
  printf("\033c");
  while (1) {
    printf("\033[1;1H");
    for (i = 0; i < MAXUSERS; i++) {
      if (bigbtmp->btmp[i].pid) {
        printf("%-3i, PID: %-5i: %-5i %-20s            \n", i,
               bigbtmp->btmp[i].pid, bigbtmp->btmp[i].usernum,
               bigbtmp->btmp[i].name);
      }
    }
    for (i = 0; i < 5; i++) {
      printf("                                   \n");
    }
    sleep(0.1);
  }
}