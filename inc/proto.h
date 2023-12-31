#pragma once
/*
 * proto.h - Prototypes of BBS functions.
 */
int checkmail(struct user *, int);
bool wanttoyell(char);
void dologout(void);

void aide_menu(void);

void deletemessage(int32_t, int32_t, bool);
int entermessage(int32_t, const char *, int8_t);
int8_t makemessage(struct user *, uint8_t, int8_t);
int readmessage(unsigned char *, bool *, char *, bool, uint32_t);

bool findroom(void);
bool forgetroom(void);
void loadroom(void);
bool nextroom(void);
void openroom(void);
void readroom(int);

int countmsgs(void);
void debug(void);
void knrooms(struct user *);
int line_more(int16_t *, int16_t);
void flush_input(int);
void fr_delete(int32_t, int32_t);
void fr_post(uint8_t, int32_t, int32_t, int32_t, struct user *);
void readdesc(void);
void storeug(int32_t *, int32_t *);
void ungoto(int, int32_t *, int32_t *);
void updatels(short *);
bool yesno(int16_t);

void change_setup(struct user *);
void doingchange(const char *);
void do_verify(struct user *, int);
void set_dob(struct user *);
void genkey(struct user *);
void dokey(struct user *);
void valkey(struct user *);

char *get_name(const char *, int);
void do_login(void);
void profile_user(int);

int telrcv(int *);
void init_states(void);

struct user *login_user(const char *, const char *);
void change_password(struct user *, const char *);
int new_user(void);

void alarmclock(void);
void init_system(void);
void logevent(const char *);
void my_exit(int);
uint16_t setup_express(void);
char *gethost(void);
int inkey(void);
int inkey_other(void);
void get_string(const char *, int, char *, int);
void get_new_string(const char *, int, char *, int, int);
int get_single_quiet(const char *);
void hit_return_now(void);
void help(const char *, int);
void more(const char *, bool);
int openfiles(void);
bool open_ip_blocklist(void);
bool check_ip_blocklist(in_addr_t);
void display_with_pagination(const char *message, uint8_t rows, int16_t);
void ip_mgmt_menu(void);
unsigned char *mmap_file(const char *path, size_t *size)
    __attribute__((nonnull(1, 2)));
unsigned char *mmap_anonymous(size_t size) __attribute__((alloc_size(1)));
int errlog(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void bbs_sleep(time_t);
void replace_substring(char *, const char *, const char *);
void cleanup_strings(char *);
void termset(void);

void locks(int);
void unlocks(int);
int add_loggedin(struct user *);
void remove_loggedin(int);
void reserve_slot(void);
void clientwho(void);
void validate_users(int);
void logout_user(struct user *, struct btmp *, int);

void show_online(int8_t);
struct user *getuser(const char *);
void freeuser(struct user *);
struct btmp *is_online(struct btmp *, struct user *, const char *);
int profile(const char *, struct user *, int);

void final_exit(void);

void checkx(int);
void xbroadcast(void);
void express(int);
void change_express(int);
void old_express(void);
void get_syself_help(int);
int xyell(struct user *, unsigned char *);
void xinit(int);
void clean_xconf(struct user *);

/* add users.c stuff */
struct user *finduser(const char *, uint32_t, int);
struct user *adduser(const char *, uint32_t);
int deleteuser(const char *);
char *getusername(const uint32_t, const int);
int getuserlink(const struct user *);
int openuser(void);
struct userdata *copyuserdata(void);
int backupuserdata(void);

/* more stuff */
void bbsstart(void);
void bbsfinger(void);
int bbssync(int);
void bbsupdate(void);
void bbsbackup(const char *);

/* queue stuff */
extern void runbbs(int16_t), dologin(int, int), drop(int), qinit(int16_t),
    setup(void), qtelrcv(int), quit(void), do_quit(void), reap(void),
    reread(void), do_reread(void), restart(void), do_restart(void), ring(void),
    dump(void), segfault(void), do_ring(void), bbsqueue(int);

extern void logfatal(const char *) __attribute__((noreturn));

/* more */
int init(void);

/* misc. */
void askansi(void);
void bcdplus1encode(int32_t number);
void change_beeps(void);
void change_doing(void);
void do_fortune(void);
void do_setup(void);
void drop(int s);
char *formtime(int how, time_t timetoform);
int colorize(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
int8_t msem_init(int32_t *, int32_t);
int32_t msem_lock(int32_t *);
int32_t msem_unlock(int32_t *);
const char *mystrstr(const char *haystack, const char *needle);
char *mystrstr_nonconst(char *haystack, const char *needle);
int my_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
int my_c_sprintf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

char *my_vsprintf(char *prefix, const char *fmt, va_list ap);
// return a new buffer that is the concatenation of 'prefix' + the formatted
// args. prefix may be null, in which case it is ignored. prefix will be free'd
// by this call and should not be used when the call returns. This always
// returns a non-null malloc'd string. caller must free.

char *my_sprintf(char *prefix, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
// return a new buffer that is the concatenation of 'prefix' + the formatted
// args. prefix may be null, in which case it is ignored. prefix will be free'd
// by this call and should not be used when the call returns. This always
// returns a non-null malloc'd string. caller must free.

int checked_snprintf_with_traceinfo(const char *file, int line, char *out,
                                    size_t, const char *fmt, ...)
    __attribute__((format(printf, 5, 6)));
// 'file' and 'line' provide tracing info.
// The remaining args are identical to snprintf, but will log
// a fatal error if the buffer would overflow.
// This is a stop-gap measure to detect overflows.
// If you hit it, it means the buffer overflow was always there, and this simply
// caught it.

#define checked_snprintf(out, len, fmt, ...)                                \
  (checked_snprintf_with_traceinfo(__FILE__, __LINE__, (out), (len), (fmt), \
                                   ##__VA_ARGS__))

char *checked_strcat_with_traceinfo(const char *file, int line, char *dest,
                                    size_t max_dest_size, const char *src);
// 'file' and 'line' provide tracing info.
// 'max_dest_size' is the total allocated capacity of the dest buffer.
// This is identical to 'strcat(dest, src)', but will log a fatal error
// if the buffer would overflow.

#define checked_strcat(dest, max_dest_size, src)                             \
  checked_strcat_with_traceinfo(__FILE__, __LINE__, (dest), (max_dest_size), \
                                (src))

int my_putchar(int c);
int my_putc(int c, FILE *stream);
int output(const char *s);
void printdate(const char *fmt);
void searchtool(void);
int ssend(int s, const char *msg, int len);
