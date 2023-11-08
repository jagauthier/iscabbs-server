#include <stdint.h>
#include <stdbool.h>
#include "defs.h"
#include "ext.h"

int main(int argc, char **argv) {
  uint8_t cmd;

  int users;

  if (openfiles() < 0) {
    my_printf("openfiles() problem!\n");
    return 1;
  }

  /* very simplistic, just takes one argument*/
  if (argc > 2 && *argv[1] == '-') {
    switch (argv[1][1]) {
      case 'u':
        users = atoi(argv[2]);
        if (users) {
          printf("Updating bigbtmp users with %i users.\n", users);
          bigbtmp->users = users;
        } else {
          printf("Must be positive.\n");
        }

        break;
      default:
        _exit(1);
    }
  }
}