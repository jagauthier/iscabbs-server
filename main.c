/*
 * Main program to start up all the various BBS pieces and parts.
 */
#include "defs.h"
#include "ext.h"

#include <err.h>

int main(int argc, char **argv) {
  char cmd = 0;

  if (chdir(ROOT "core/bbs") == -1) {
    err(EXIT_FAILURE, "failed to chdir to '%s'", ROOT "core/bbs");
  }

  umask(027);

  if (openfiles() < 0) {
    my_printf("openfiles() problem!\n");
    return (-1);
  }

  if (argc < 2 || *argv[1] != '-') {
    cmd = BBS;
  } else {
    switch (argv[1][1]) {
      case 'b':
        if (argc == 3) {
          cmd = BACKUP;
        }
        break;

      case 'f':
        cmd = FINGER;
        break;

      case 'i':
        cmd = INIT;
        break;

      case 's':
        cmd = SYNC;
        break;

      case 'q':
        cmd = QUEUE;
        break;

      case 'u':
        cmd = UPDATE;
        break;

      default:
        _exit(1);
#ifdef _WHIP
        __xxx(0);
#endif
        break;
    }
  }
  
  for (;;) {
    if (cmd == BBS || cmd == UPDATE) {
      setgid(BBSGID);
      setuid(BBSUID);
    }

    switch (cmd) {
      case BBS:
        ARGV = argv;
        bbsstart();
        break;

      case FINGER:
        bbsfinger();
        break;

      case INIT:
        cmd = bbssync(1);
        continue;

      case SYNC:
        bbssync(0);
        break;

      case UPDATE:
        nice(40);
        bbsupdate();
        break;

      case BACKUP:
        nice(40);
        bbsbackup(argv[2]);
        break;

      case QUEUE:
#if 0
        bbsqueue(getenv("INIT_STATE") ? 0 : 1);
	break;
#else
        bbsqueue(1);
        break;
#endif

      default:
        break;
    }
    break;
  }

  return (0);
}
