/*
 * Support for a finger daemon that shows the BBS who list.
 */
#include "defs.h"
#include "ext.h"

#define MAXCONN 60

static void setup_socket(void) {
  struct sockaddr_in sa;
  int one = 1;

  close(0);
  if (socket(AF_INET, SOCK_STREAM, 0) != 0) {
    _exit(1);
  }
  one = 1;
  if (setsockopt(0, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0) {
    _exit(1);
  }
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = htonl(0);
  sa.sin_port = htons(79);
  if (bind(0, (const struct sockaddr *)&sa, sizeof sa) < 0) {
    _exit(1);
  }
  if (listen(0, SOMAXCONN) < 0) {
    _exit(1);
  }
}

static void s_sigalrm2(int signo) {
  if (signo == SIGALRM) {
    signal(SIGALRM, s_sigalrm2);
    alarm(30);
  }
}

void bbsfinger_old(void) {
  int8_t n;
  int i;
  int x;
  char *p;
  fd_set rfd;
  struct fd {
    int16_t conn;
    int16_t pos;
    char buf[24];
  } fd[MAXCONN];

  // if (fork()) {
  //   printf("Exit.\n");
  //   _exit(0);
  // }
  nice(-20);
  nice(-20);
  nice(20);

  //close(0);
  //close(1);
  //close(2);
  setsid();

  signal(SIGPIPE, SIG_IGN);
  signal(SIGALRM, s_sigalrm2);
  alarm(30);

  ouruser = getuser("Guest");

  if (!ouruser) {
    printf("!ouruser\n");
    _exit(1);
  }

  rows = 32000;

  setup_socket();

  const size_t bigstdoutbuf_size = 262143;
  char *bigstdoutbuf = calloc(bigstdoutbuf_size, sizeof(char));
  setvbuf(stdout, bigstdoutbuf, _IOFBF, bigstdoutbuf_size);

  for (;;) {
    FD_ZERO(&rfd);
    for (x = 0, i = 3; i < MAXCONN; i++) {
      if (fd[i].conn) {
        FD_SET(i, &rfd);
        x = i;
      } else {
        FD_SET(0, &rfd);
      }
    }
    if ((i = select(x + 1, &rfd, 0, 0, 0)) < 0 && errno != EINTR) {
      printf("FINGER: select error %d\n", errno);
      perror("finger");
      _exit(1);
    } else if (!i) {
      printf("FINGER: 0 return from select\n");
      continue;
    } else if (i < 0) {
      for (i = 2; i < MAXCONN; i++) {
        if ((fd[i].conn > 0 && ++fd[i].conn > 4) ||
            (fd[i].conn < 0 && --fd[i].conn < -4)) {
          close(i);
          fd[i].conn = 0;
        }
      }
      if (fd[0].conn++ > 5) {
        setup_socket();
      }

      continue;
    }

    while (FD_ISSET(0, &rfd)) {
      struct sockaddr_in sa;
      struct linger linger;

      fd[0].conn = 0;
      socklen_t size = sizeof sa;
      if ((i = accept(0, (struct sockaddr *)&sa, &size)) < 0) {
        break;
      }
      if (i != 1) {
        printf("FINGER: accepted on fd %d\n", i);
        close(i);
        break;
      }

      linger.l_onoff = 1;
      linger.l_linger = 0;
      if (setsockopt(1, SOL_SOCKET, SO_LINGER, &linger, sizeof linger) < 0) {
        close(1);
        break;
      }

      size = bigstdoutbuf_size;
      if (setsockopt(1, SOL_SOCKET, SO_SNDBUF, &size, sizeof size) < 0) {
        close(1);
        break;
      }

      if (fcntl(1, F_SETFL, O_NONBLOCK) < 0) {
        close(1);
        break;
      }

      i = dup(1);
      close(1);
      fd[i].conn = 1;
      fd[i].pos = 0;
      break;
    }

    for (n = 2; n < MAXCONN; n++) {
      if (!FD_ISSET(n, &rfd)) {
        continue;
      }
      if (fd[n].conn < 0 ||
          (i = read(n, fd[n].buf + fd[n].pos, sizeof fd[n].buf - fd[n].pos)) <=
              0 ||
          (fd[n].pos += i) == sizeof fd[n].buf) {
        close(n);
        fd[n].conn = 0;
        continue;
      }

      if (fd[n].pos < 2 || strncmp(fd[n].buf + fd[n].pos - 2, "\r\n", 2)) {
        continue;
      }
      fd[n].buf[fd[n].pos - 2] = 0;

      dup2(n, 1);

#ifdef _WHIP
      if (!strcmp(fd[n].buf, "__FULL__")) {
#else
      if (!strncmp(fd[n].buf, "/W", 2) || *fd[n].buf == '@') {
#endif
        show_online(2);
      } else if (!*fd[n].buf) {
        show_online(3);
      } else {
        for (x = 1, p = fd[n].buf; *p; p++) {
          if (x && islower(*p)) {
            *p = toupper(*p);
            x = 0;
          } else if (*p == '_' || *p == ' ')
            x = (*p = ' ') != 0;
          else {
            x = 0;
          }
        }
        if (profile(fd[n].buf, NULL, PROF_REG) < 0) {
          printf("There is no user %s on this BBS.\n", fd[n].buf);
        }
      }

      fd[n].conn = -1;
      fflush(stdout);
      close(1);
      shutdown(n, 1);
    }
  }

  free(bigstdoutbuf);
}
