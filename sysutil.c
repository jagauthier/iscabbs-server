#include "defs.h"
#include "ext.h"

static void myecho(int mode);

static void s_sigdie() {
    if (tty) {
        signal(SIGHUP, (void *)s_sigdie);
    }
    signal(SIGPIPE, (void *)s_sigdie);
    signal(SIGTERM, (void *)s_sigdie);
    if (f_death > -2) {
        my_exit(0);
    }
}

static void s_sigquit() {
    if (!tty) {
        signal(SIGHUP, (void *)s_sigquit);
    }
    signal(SIGQUIT, (void *)s_sigquit);
    if (!f_death) {
        f_death = 1;
    }

    // errlog ("SIGQUIT while lock on %d", lockflags);
}

static void s_sigio(void) {
    /* Force-check */
    if (XPENDING) {
        checkx(1);
    }

    signal(SIGIO, (void *)s_sigio);
}

static void s_sigusr2(void) {
    int i;

    for (i = 8; i > 0; i--) {
        my_putchar(BEL);
        fflush(stdout);
        usleep(130000);
    }
    signal(SIGUSR2, (void *)s_sigusr2);
}

static void s_sigalrm(void) {
    signal(SIGALRM, (void *)s_sigalrm);

    /* if (f_alarm || f_death == -1 || dead == 1)  - Why? */
    if (f_death == -1 || dead == 1) {
        my_exit(0);
    } else if (f_alarm) {
        alarmclock();
    } else if (!f_death) {
        f_alarm = 1;
        alarm(120);
    }
}

void alarmclock(void) {
    f_alarm = 0;
    logintime += 5;
    mybtmp->sleeptimes += 5;
    alarm(mybtmp->time + (logintime + 5) * 60 - (msg->t = time(0)));
    xcount = xcount > logintime * 4 ? xcount : logintime * 4;
    postcount = postcount > logintime * 2 ? postcount : logintime * 2;

    if (!ouruser && nonew >= 0) {
        colorize(
            "\n\n\a" BOLD_CYAN
            "You have exceeded the time limit for entering your name and "
            "password.\n\n");
        my_exit(10);
    } else if (nonew < 0 && logintime == 30) {
        colorize(
            "\n\n\a" BOLD_CYAN
            "You have exceeded the time limit for entering your "
            "information.\n\n");
        my_exit(10);
    }

#ifdef TOTALTIMELIMIT

    if (logintime == 6000) {
        colorize(
            "\n\n\a" BOLD_CYAN
            "You have exceeded the maximum login time.  Please call back "
            "later!\n\n");
        my_putchar(BEL);
        my_exit(10);
    } else if (logintime == 5985) {
        colorize("\n\n\a" BOLD_YELLOW "You have 15 minutes left before you are logged out!\n\n");
        fflush(stdout);
    } else if (logintime == 5995) {
        colorize("\n\n\a" BOLD_YELLOW "You have 5 minutes left before you are logged out!\n\n");
        fflush(stdout);
    }

#endif

#ifdef IDLELOGOUT
    if (!client && !ouruser->f_aide && !ouruser->f_prog &&
        mybtmp->sleeptimes >= IDLELOGOUT) {
        colorize("\n\nYou have been logged out for inactivity.\n\n");
        my_exit(10);
    }
#endif

    if (client && mybtmp->sleeptimes >= 15) {
        dead = 1;
        my_putchar(IAC);
        my_putchar(CLIENT);
        fflush(stdout);
    }

    if (ouruser && !guest && !(logintime % 30))
        msync((caddr_t)ouruser, sizeof(struct user), MS_ASYNC);
}

/*
 * Initialize the system.
 */
void init_system(void) {
    struct sigaction sact;

    tty = isatty(0);
    pid = getpid();

    signal(SIGTSTP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGIO, (void *)s_sigio);
    signal(SIGALRM, (void *)s_sigalrm);
    // signal(SIGQUIT, (void *)s_sigquit);
    signal(SIGTERM, (void *)s_sigdie);
    signal(SIGPIPE, (void *)s_sigdie);
    signal(SIGUSR2, (void *)s_sigusr2);
    signal(SIGHUP, tty ? (void *)s_sigdie : (void *)s_sigquit);

    alarm(300);

    bzero(&sact, sizeof(sact));
    sact.sa_handler = (void *)s_sigquit;
    sact.sa_flags = 0;

    if (sigaction(SIGQUIT, &sact, NULL)) {
        perror("sigaction");
    }

    if (!strncmp(*ARGV, "_clientbbs", 10)) {
        client = 1;
    }

    if (!tty) {
        init_states();
    }

    if (tty) {
        myecho(OFF);
    }

    lastbcast = msg->lastbcast;
}

void logevent(const char *message) {
    int f;

    if ((f = open(LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0640)) >= 0) {
        time_t t = msg->t = time(0);
        struct tm *tp = localtime(&t);
        char *buf =
            my_sprintf(NULL, "%02d%02d%02d:%02d%02d %s : %s\n", tp->tm_year % 100,
                       tp->tm_mon + 1, tp->tm_mday, tp->tm_hour, tp->tm_min,
                       ouruser ? ouruser->name : "_NEWUSER_", message);
        write(f, buf, strlen(buf));
        close(f);
        free(buf);
    }
}

/*
 * Exit the program in an orderly way.
 */
void my_exit(int doflush) {
    int save = f_death;

    if (lockflags) {
        errlog("lockflags=%d", lockflags);
        f_death = 2;
        return;
    }

    f_alarm = 0;
    if (doflush) {
        f_death = -1;
    } else {
        f_death = -2;
    }
    alarm(120);

    if (mybtmp) {
        mybtmp->nox = -1;
    }

    if (save == 1 && doflush) {
        my_printf("\a\n\n\n\n\nYou have been logged off.\n\n\n\n\n");
    }

    if (ouruser) {
        int f;
        struct tm *ltm;

        if (doflush) {
            checkx(-1);
        }

        locks(SEM_USER);
        if (ouruser->btmpindex >= 0 &&
            mybtmp == &bigbtmp->btmp[ouruser->btmpindex]) {
            ouruser->btmpindex = -1;
        }
        unlocks(SEM_USER);
        ouruser->timeoff = msg->t = time(0);
        ouruser->timetot = ouruser->timetot + ouruser->timeoff - ouruser->time;

        if ((f = open(ETC "uselog", O_WRONLY | O_CREAT | O_APPEND, 0640)) >= 0) {
            ltm = localtime(&ouruser->time);
            char *junk = my_sprintf(
                NULL, "%02d%02d%02d:%02d%02d:%04ld:%s\n", ltm->tm_year % 100,
                ltm->tm_mon + 1, ltm->tm_mday, ltm->tm_hour, ltm->tm_min,
                (ouruser->timeoff - ouruser->time) / 60 + 1, ouruser->name);
            write(f, junk, strlen(junk));
            close(f);
            free(junk);
        }

        msync((caddr_t)ouruser, sizeof(struct user), MS_ASYNC);
        ouruser = NULL;
    }

    if (bigbtmp) {
        remove_loggedin(pid);
        bigbtmp = 0;
    }

    if (doflush) {
        if (ansi) {
            colorize(ANSI_RESET);
        }
        fflush(stdout);
        if (doflush > 1) {
            bbs_sleep(doflush);
        }
        if (tty) {
            myecho(ON);
        } else {
            int fdr = 1;
            struct timeval tv = {30, 0};

            shutdown(0, 1);
            select(1, (fd_set *)&fdr, 0, 0, &tv);
        }
    }

#ifdef _SSL
    if (ssl) SSL_free(ssl);
#endif

    _exit(0);
}

static void myecho(int mode) {
    struct termios term;

    tcgetattr(1, &term);
    if (!saveterm.c_lflag && !saveterm.c_iflag) {
        memcpy((char *)&saveterm, (const char *)&term, sizeof(struct termios));
    }

    if (!mode) {
        term.c_lflag &= ~(ICANON | ECHO | ECHOK | ECHOE | ECHONL | ISIG);
        term.c_iflag |= IXOFF;
        term.c_cc[VMIN] = 1;
        term.c_cc[VTIME] = 0;
    } else {
        memcpy((char *)&term, (const char *)&saveterm, sizeof(struct termios));
    }

    tcsetattr(1, TCSANOW, &term);
}

/*
 * inkey() returns with the next typed character.
 *
 * It has a built-in timeout, giving the user a 1 minute warning then logging
 * them out.
 */

int inkey(void) {
    int i = 257;
    int noflush = 1;

    while (i > DEL) {
        if (!tty) {
            if ((i = telrcv(&noflush)) < 0) {
                my_exit(0);
            }
            if (block) {
                i = 257;
            } else if (i != 17) {
                byte++;
            }
        } else if (!INPUT_LEFT()) {
            if (noflush) {
                fflush(stdout);
                noflush = 0;
            }
            do {
                if (f_alarm) {
                    alarmclock();
                }
                if (f_death) {
                    my_exit(5);
                }
                if (XPENDING) {
                    checkx(1);
                }
            } while ((i = getchar()) < 0 && errno == EINTR);

            if (i < 0) {
                my_exit(0);
            }
        } else {
            i = getchar();
        }
        if (lastcr && i == '\n' && !client) {
            i = 255;
        }
        lastcr = 0;
    }

    /* Change a couple values to the C/Unix standard */
    if (i == DEL) {
        i = BS;
    } else if (i == '\r') {
        i = '\n';
        lastcr = 1;
    } else if (i == CTRL_U) {
        i = CTRL_X;
    }

    /* reset the sleep time *after* they type a letter, not after the command
     is executed */
    mybtmp->sleeptimes = 0;
    return (i);
}

void printdate(const char *fmt) {
    time_t t;

    t = msg->t = time(0);
    my_printf(fmt, ctime(&t));
}

/*
 * get_string (prompt, length, result, line)
 *
 * Display the prompt given and then accept a string with the length specified.
 * Beeps if user tries to enter too many characters.  Backspaces are allowed.
 * Will allow blank entries.
 *
 */

void get_string(const char *prompt, int length, char *result, int line) {
    get_new_string(prompt, length, result, line, 5);
}

void get_new_string(const char *prompt, int length, char *result, int line,
                    int limit) {
    static char wrap[80];
    char *rest;
    char *p = result;
    char *q;
    int c;
    int hidden;
    int invalid = 0;

    my_printf("%s", prompt);
    if (line <= 0) {
        *wrap = 0;
    } else if (*wrap) {
        my_printf("%s", wrap);
        strcpy(result, wrap);
        p = result + strlen(wrap);
        *wrap = 0;
    }

    if (client && length < 78) {
        my_putchar(IAC);
        my_putchar(G_STR);
        my_putc(length, stdout);
        my_putc((byte >> 16) & 255, stdout);
        my_putc((byte >> 8) & 255, stdout);
        my_putc(byte & 255, stdout);
        block = 1;
    }

    hidden = 0;
    if (length < 0) {
        hidden = length = 0 - length;
    }
    for (;;) {
        c = inkey();
        if (c == ' ' && length == 29 && p == result) {
            break;
        }
        if (c == '\n') {
            break;
        }

        if (c < SP && c != BS && c != CTRL_X && c != CTRL_W && c != CTRL_R) {
            if (invalid++) {
                flush_input(invalid < 6 ? (invalid / 2) : 3);
            }
            continue;
        } else {
            invalid = 0;
        }

        if (c == CTRL_R) {
            *p = 0;
            if (!hidden) {
                my_printf("\n%s", result);
            }
            continue;
        }

        if (c == BS || c == CTRL_X) {
            if (p == result) {
                continue;
            } else {
                do {
                    my_putchar(BS);
                    my_putchar(SP);
                    my_putchar(BS);
                    --p;
                } while (c == 24 && p > result);
            }
        } else if (c == CTRL_W) {
            for (q = result; q < p; q++) {
                if (*q != ' ') {
                    break;
                }
            }
            for (c = q == p; p > result && (!c || p[-1] != ' '); p--) {
                if (p[-1] != ' ') {
                    c = 1;
                }
                my_putchar(BS);
                my_putchar(SP);
                my_putchar(BS);
            }
        } else if (p < result + length && c >= SP) {
            *p++ = c;
            if (!client) {
                if (!hidden) {
                    my_putchar(c);
                } else {
                    my_putchar('.');
                }
            }
        } else if (c < SP || line < 0 || line == limit - 1) {
            continue;
        } else {
            if (c == ' ') {
                break;
            }
            for (q = p - 1; *q != ' ' && q > result; q--)
                ;
            if (q > result) {
                *q = 0;
                for (rest = wrap, q++; q < p;) {
                    *rest++ = *q++;
                    my_putchar(BS);
                    my_putchar(SP);
                    my_putchar(BS);
                }
                *rest++ = c;
                *rest = 0;
            } else {
                wrap[0] = c;
                wrap[1] = 0;
            }
            break;
        }
    }
    *p = '\0';
    if (!client) {
        my_putchar('\n');
    }
}

int get_single_quiet(const char *valid_string) {
    int c;
    int invalid = 0;

    for (;;) {
        c = inkey();
        /* First check it in the case given */
        if (index(valid_string, c)) {
            break;
        }
        /* If not, if we're lower case, try upper case */
        if (c >= 'a' && c <= 'z') {
            c -= 32;
            if (index(valid_string, c)) {
                break;
            }
        }
        /* It is an invalid character... */
        if (invalid++) {
            flush_input(invalid < 6 ? (invalid / 2) : 3);
        }
    }
    return (c);
}

void hit_return_now(void) {
    flush_input(0);
    my_printf("\nHit return to continue...");
    get_single_quiet("\n");
    my_putchar('\n');
}

/*
 * MORE
 *
 * Put a file out to the screen, stopping every 24 lines.  Like the unix more
 * utility.
 */

void more(const char *filename, bool comments) {
    int16_t line;
    size_t size;
    unsigned char *filep;
    unsigned char *p;
    size_t i;
    bool noprint;

    my_putchar('\n');
    size = 0;
    if (!(filep = p = mmap_file(filename, &size))) {
        errlog("File %s is missing (%d rows)", filename, rows);
        my_printf("File %s is missing, sorry!\n\n", filename);
        return;
    }

    for (noprint = comments && *p == '#', line = i = 0; i < size; i++, p++)
        if ((noprint ? *p : my_putchar(*p)) == '\n') {
            if (!noprint && ++line >= rows - 1 &&
                line_more(&line, (int16_t)((i * 100) / size)) < 0) {
                break;
            } else {
                noprint = comments && p[1] == '#';
            }
        }

    my_putchar('\n');
    munmap((void *)filep, size);
}

void bbs_sleep(time_t sec) {
    struct timeval tv;
    time_t t;

    fflush(stdout);
    t = time(0);
    tv.tv_sec = sec;
    tv.tv_usec = 0;
    while (tv.tv_sec > 0 && select(1, 0, 0, 0, &tv) < 0 && errno == EINTR) {
        if (f_death) {
            break;
        }
        if (f_alarm) {
            alarmclock();
        }
        if (tv.tv_sec == sec && !tv.tv_usec) {
            tv.tv_sec -= time(0) - t;
        }
        t = time(0);
    }
}

int errlog(const char *fmt, ...) {
    int fd;

    if ((fd = open(ERRLOG, O_WRONLY | O_CREAT | O_APPEND, 0640)) < 0) {
        return (-1);
    }

    // write ctime to buffer
    time_t t = time(0);
    char *buf = my_sprintf(NULL, "%s", ctime(&t));

    // remove the new line
    buf[strlen(buf) - 1] = 0;

    // append name
    buf = my_sprintf(buf, " (%s): ", ouruser ? ouruser->name : "__NONE__");

    // append formatted args
    va_list ap;
    va_start(ap, fmt);
    buf = my_vsprintf(buf, fmt, ap);
    va_end(ap);

    // append newline
    buf = my_sprintf(buf, "\n");

    write(fd, buf, strlen(buf));  // TODO: check return
    close(fd);

    free(buf);

    return 0;
}

static size_t file_size_from_fd(int fd) {
    struct stat sb;
    memset(&sb, 0, sizeof(struct stat));
    if (fstat(fd, &sb) == 0) {
        return sb.st_size;
    } else {
        return 0;
    }
}

unsigned char *mmap_file(const char *path, size_t *size) {
    // open the fd
    int fd = open(path, O_RDWR);
    if (fd == -1) {
        return NULL;
    }

    // get the current size of the file.
    size_t fsz = file_size_from_fd(fd);

    // The caller requested `*size` bytes.
    if (*size > fsz) {
        close(fd);
        return NULL;
    }

    // zero means they want the whole file.
    if (*size == 0) {
        *size = fsz;
    }

    // mmap, close, then validate.
    void *p =
        mmap(NULL, *size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, fd, 0);
    close(fd);
    if (p == MAP_FAILED) {
        return NULL;
    }

    return (unsigned char *)p;
}

unsigned char *mmap_anonymous(size_t size) {
    void *p = mmap(NULL, size, PROT_READ | PROT_WRITE,
                   MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (p == MAP_FAILED) {
        return NULL;
    }

    return (unsigned char *)p;
}
