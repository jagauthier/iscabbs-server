extern struct room     sroom;
extern struct room    *room;
extern uint16_t	       curr;
extern long            savedhighest;
extern struct btmp    *mybtmp;
extern struct bigbtmp *bigbtmp;
extern struct blocklist *blocklist;
extern struct user    *ouruser;	/* The person we have logged in here */
extern struct voteinfo	*voteinfo;	/* Voting booth */
extern int16_t           logintime;
extern char            sysopflags;
extern pid_t           pid;		/* pid of this process */
extern char            pbuf[MAXNAME + 1];
extern char            profile_default[MAXALIAS + 1];	/* Default name to profile */
extern char	       xreply[MAXALIAS + 1];
extern char            to[MAXALIAS + 1];
extern char            nonew;		/* No new users accepted this site */
extern char          **ARGV;
extern struct termios  saveterm;
extern char            tty;
extern char            client;		/* Is connected to a BBS client */
extern char            block;
extern int32_t             byte;
extern int16_t           dead;		/* True if the client might be dead */
extern time_t          lastwho;
extern int16_t           postcount;
extern int16_t           xcount;
extern int32_t             numposts;
extern char            guest;
extern char            lastcr;
extern unsigned char  *msgstart;
extern unsigned char  *tmpstart;
extern struct msg     *msg;
extern unsigned char  *xmsg;
extern int16_t           xmsgnum;
extern char            curr_syself[MAXALIAS + 1];
extern unsigned char   skipping[(MAXROOMS >> 3) + 1];
extern char            ansi;
extern int16_t           rows;
extern bool	       alt;

extern int32_t	       randcnt;
extern int32_t 	       randchar;

extern char            state;

extern unsigned char   subbuffer[8];
extern unsigned char  *subpointer;
extern unsigned char  *subend;

extern char lockflags;
extern char f_death;
extern char f_alarm;
extern char f_qalarm;
extern char f_reread;
extern char f_term;
extern char f_quit;
extern char f_restart;
extern time_t lastbcast;
extern struct bigbtmp *q;
extern volatile char foo;

// Use this instead of MM_FILELEN
extern int32_t mm_filelen;

#ifdef extern
const char months[12][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
#else
extern const char months[12][4];
#endif

extern int sfd;		/* Socket fd */
#ifdef _SSL
extern SSL *ssl;
#endif
