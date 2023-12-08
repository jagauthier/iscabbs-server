/*
 * bbs.h - Defines specific to the BBS.
 */


/* Configuration constants */
#define BBSNAME	     "ISCABBS"
#define BBSUID       1001               /* UID of owner of all BBS files    */
#define BBSGID       1001               /* GID of owner of all BBS files */
#define NXCONF       60			/* Number of slots in X config */
#define MAILMSGS     200		/* Number of mail messages per user */
#define MSGSPERRM    200		/* Messages per room (>= MAILMSGS!!) */
#define MAXROOMS     200		/* Number of rooms the BBS will have */
#define MAXUSERS     200                /* Maximum users online at a time */
#define MAXNEWBIES   2000               /* Maximum number of pending newbies */
#define MAXQ         1000               /* Maximum queue */
#define MM_FILELEN   (200000000)		/* size of msgmain */
#define XMSGSIZE     (50000000)		/* size of msgmain */
#define MAXVOTES     64			/* Max number of voting items */

/* System wide file constants */

#define ROOT        "/home/bbs/"             /* root directory of all BBS stuff */

#ifdef _SSL
#define SSLCERTIFICATE	ROOT"sslprivate"
#define SSLKEYPATH	ROOT"sslprivate"
#endif


#define AIDELIST    ROOT"etc/aidelist"	/* copy of who's an aide    */
#define NEWAIDELIST    ROOT"etc/newaidelist"	/* copy of who's an aide    */
#define ERRLOG      ROOT"etc/errlog"	/* Error log file */
#define ETC         ROOT"etc/"		/* Where lots of stuff resides */
#define FORTUNE	    "/usr/bin/fortune -a"	/* Fortune! */
#define HELPDIR     ROOT"help/"		/* Location of help files    */
#define LIMITS      ROOT"etc/limits"	/* login limits for the BBS */
#define LOCKOUT     ROOT"etc/lockout"	/* Sites that are locked out */
#define LOGFILE     ROOT"etc/log"	/* System log                */
#define DESCDIR     ROOT"message/desc/" /* location of room info files */
#define MESSAGE     ROOT"message/"	/* Message directory     */
#define MSGDATA     ROOT"data/msgdata"	/* Message system (non temp) data */
#define MSGMAIN     ROOT"message/msgmain"/* Main message file */
#define TMPDATA     ROOT"data/tmpdata"	/* Temp data (not saved to disk) */
#define USERDATA    ROOT"data/userdata"	/* User index/data */
#define XMSGDATA    ROOT"data/xmsgdata" /* X message data file */
#define MOTD	    ROOT"etc/motd"	/* Message of the day */
#define VOTEFILE    ROOT"etc/votedata"	/* Voting info */
#define WHODIR      ROOT"etc/who/"	/* who knows rooms directory */
#define ISCAWHO      ROOT"etc/iscawho"	/* who knows rooms directory */


/* Miscellaneous Defs */

#define ABORT           2
#define YELLS_RM_NBR		2
#define SYSOP_RM_NBR		6
#define DELMSG_RM_NBR		3
#define BACKWARD		(-1)
#define BEL                     7	/* control-g (bell) */
#define BS			'\b'	/* back space */
#define CTRL_D                  4	/* control-d */
#define CTRL_R			18	/* control-r */
#define CTRL_U			21	/* control-u */
#define CTRL_W			23	/* control-w */
#define CTRL_X                  24	/* control-x */
#define DEL			127	/* delete character */
#define FF			255	/* == 255, begin D.O.C. msg. */
#define FMTERR                  1	/* format error in D.O.C. file */
#define FORWARD			1
#define LF			'\n'	/* line feed (new line) */
#define LOBBY_RM_NBR		0
#define MAIL_RM_NBR		1
#define MARGIN                  80
#define MAXALIAS		19	/* Max size for logon names */
#define MAXNAME                 40	/* Max size for room names */
#define MES_NORMAL	        65	/* Normal message                   */
#define MES_ANON	        66	/* "****" header                    */
#define MES_AN2		        67	/* "Anonymous" header               */
#define MES_DESC                68	/* Room description "message"       */
#define MES_SYSOP		69	/* Message from sysop		    */
#define MES_XYELL		70	/* Yell of harassing X's pasted in  */
#define MES_FM			71	/* Message from FM		    */
#define MMAP_ERR		3	/* Error mmapping temp space        */
#define MNFERR                  666	/* Message not found error          */
#define REPERR			667	/* Message repetition "error"       */
#define NL			LF
#define NOISY                   0
#define PROF_REG		0	/* Regular profile */
#define PROF_EXTRA		1	/* Show extra info on others */
#define PROF_SELF		2	/* Self profile */
#define PROF_ALL		3	/* Show all info (sysop) */
#define QUIET           1
#define REVERSE		(-1)
#define QR_INUSE	2		/* Set if in use, clear if avail    */
#define QR_PRIVATE	4		/* Set for any type of private room */
#define QR_GUESSNAME	16		/* Set if it's a guessname room     */
#define QR_PASSWORD	32		/* Set if its a passworded          */
#define QR_ANONONLY	64		/* Anonymous-Only room              */
#define QR_ANON2	128		/* Anonymous-Option room            */
#define SAVE                    0
#define SP			32	/* space character */
#define SYSOP_MSG		1	/* Last message was from sysop */
#define SYSOP_FROM_SYSOP	2	/* Flag sysop post from sysop  */
#define SYSOP_FROM_USER		4	/* Flag sysop post from user   */
#define SYSOP_FROM_FM		8	/* Flag "sysop" post from FM   */
#define TWILIGHTZONE            (-1L) /* (-1L) Zapped room (?) */
#define RODSERLING              (-2L) /* (-2L) kicked out of public  room */
#define NEWUSERFORGET           (-3L) /* (-3L) so newbies aren't overwhelmed!*/
#define TAB			9


// the below is only used in update.c, but it's in here due to being used
// in multiple functions there, unless there's a better way to do it that
// I find, it's staying in here

struct room_glob {
  char roomname[MAXNAME + 1];
  int32_t roomnumber;
  char moderator_name[200]; //[MAXALIAS + 1];
  uint32_t moderator_number;
};



struct user
{
  uint32_t   usernum;			/* User's Citadel user num   */

  uint32_t an_www:1;		/* WWW address withheld */
  uint32_t an_site:1;		/* Connection site withheld */
  uint32_t an_mail:1;		/* Mail withheld */
  uint32_t an_phone:1;		/* Phone withheld */
  uint32_t an_location:1;		/* Location (city/state/zip) withheld*/
  uint32_t an_addr:1;		/* Address withheld */
  uint32_t an_name:1;		/* Name withheld */
  uint32_t an_all:1;		/* All withheld */

  uint32_t f_duplicate:1;		/* Has duplicate/multiple accounts */
  uint32_t f_admin:1;		/* (prog, aide) */
  uint32_t f_restricted:1;		/* (badinfo, duplicate, etc.) */
  uint32_t f_prog:1;		/* Is a programmer */
  uint32_t f_badinfo:1;		/* Marked as having bad address info */
  uint32_t f_newbie:1;		/* Is a new user */
  uint32_t f_inactive:1;		/* Marked as inactive */
  uint32_t f_deleted:1;		/* Marked for deletion */

  uint32_t f_ownnew:1;		/* See own posts as new */
  uint32_t f_namechanged:1;		/* Has had name changed */
  uint32_t f_noanon:1;		/* Not permitted to make anon posts */
  uint32_t f_noclient:1;		/* Not permitted to use BBS client */
  uint32_t f_ansi:1;		/* Wants ANSI query at login */
  uint32_t f_trouble:1;		/* New user from a "trouble" site */
  uint32_t f_invisible:1;		/* User is invisible to normal users */
  uint32_t f_autoelf:1;		/* Guide flag enabled upon login */

  uint32_t f_novice:1;		/* Is an "novice" user */
  uint32_t f_elf:1;			/* Is a guide */
  uint32_t f_nobeep:1;		/* Have X's not cause a beep */
  uint32_t f_xmsg:1;		/* Have X's arrive while busy */
  uint32_t f_ampm:1;		/* Show time in AM/PM format */
  uint32_t f_shortwho:1;		/* Show short wholist by defaut */
  uint32_t f_twit:1;		/* Is a twit */

  uint32_t f_beeps:1;		/* allow BEEPS keyword */
  uint32_t f_lastold:1;		/* Show 1 old post when reading new */
  uint32_t f_xoff:1;		/* X's off by default */
  uint32_t f_clear:1;		/* Mark posts old after any read new */
  uint32_t f_aide:1;		/* Is a sysop */
  uint32_t f_revwho:1;		/* Reverse the order of the wholist */

  int32_t     timescalled;                  /* how many calls */
  int32_t     posted;                       /* number of msgs posted */
  time_t  time;                         /* Time of last login        */
  time_t  timeoff;                      /* Time of last logout */
  time_t  timetot;                      /* Total time online for day */
  time_t  firstcall;			/* When user acct was created */
  char    name[MAXALIAS + 1];		/* Login name                */
  char    passwd[14];			/* the user's crypted passwd */
  char    remote[40];			/* how connected             */
  char    real_name[MAXNAME + 1];	/* User's real name          */
  char    addr1[MAXNAME + 1];		/* Home address 1 of 2       */
  char    addr2[MAXNAME + 1];
  char    city[21];			/* User's city               */
  char    state[21];			/* Users's state or country */
  char    zip[11];			/* Zipcode or mailcode        */
  char    phone[21];			/* Phone in format (AAA)-NNN-NNNN */
  char    mail[41];			/* User's internet mail address */
  char    desc1[81];			/* User description 1 of 5   */
  char    desc2[81];
  char    desc3[81];
  char    desc4[81];
  char    desc5[81];
  int8_t    generation[MAXROOMS];		/* generation # flags joined rooms */
  int8_t    forget[MAXROOMS];		/* generation # flags forgotten rooms */
  int32_t    lastseen[MAXROOMS];           /* last message seen in each room */
  int32_t	  quickx[10];			/* 10 slots for QuickX */
  struct xconf
  {
    uint32_t :7;
    uint32_t which:1;		/* 0 = disable, 1 = enable   */
    uint32_t usernum:24;		/* User to enable/disable*/
  } xconf[NXCONF];
  char www[60];
  time_t xconftime;			/* Modification time of xconf */
  struct usermail			/* User's private mail room */
  {
    int32_t    num;			/* Universal message number          */
    int32_t    pos;			/* Message positions in master file  */
  } mr[MAILMSGS];
  char    aideinfo[81];			/* Just for the Sproinglet */
  char    reminder[81];			/* remind yourself of whatever */
  char    loginname[16];		/* New larger field for loginname */
  int8_t    unused0;			/* (WAS) Anonymous aide/wizard */
  int32_t    totalx;			/* Number of X's sent */
  char    A_real_name[MAXNAME + 1];	/* User's real name */
  char    A_addr1[MAXNAME + 1];		/* Home address */
  char    A_city[21];			/* User's city */
  char    A_state[21];			/* Users's state or country */
  char    A_zip[11];			/* Zipcode or mailcode */
  char    A_phone[21];			/* Phone in format (AAA)-NNN-NNNN */
  char    A_mail[41];			/* User's internet mail address */
  char    doing[40];			/* What the hell are you doing? */
  char    vanityflag[40];		/* Have a nice vanity flag */
  int16_t   btmpindex;			/* Index to user's record in btmp */
  int32_t    xseenpos;
  int32_t    xmaxpos;
  int16_t   xmsgnum;
  int32_t    xminpos;
/* remember alignment issues when adding more */
  uint16_t yells;			/* Yells answered as sysop */
  uint16_t vals;			/* Validations done as sysop */
};


struct bigbtmp
{
  int32_t users;			/* Users online now */
  int32_t queued;			/* Users queued now */
  int16_t index[MAXUSERS];	/* Indices into btmp struct array */
  time_t ghostcheck;		/* Cheap hack */
  int32_t eternal;			/* Eternal number (another check hack) */

  int16_t qp;			/* Index to end of queue */
  int16_t oldqp;
  int16_t maxqp;			/* Maximum users in queue so far */
  int16_t socks;			/* Total sockets connected at this time */
  int32_t forks;			/* Total number of forks so far */
  int32_t reaps;			/* Total number of reaps so far */
  int16_t connectable;		/* Allowable connects during next 30 seconds */
  int16_t startable;		/* Allowable forks during next 30 seconds */
  int32_t aidewiz;			/* Number of aide/wizard logins */
  int32_t upgrade;			/* Number of upgraded user logins */
  int32_t nonupgrade;		/* Number of regular user logins */
  int16_t starts;			/* Forks in last 30 second period */
  int16_t qflag;			/* Set when connection dropped */
  char hello[1000];		/* Hello message */
  int8_t unused[16];		/* Space made as unused */
  int8_t sig_alrm;
  int8_t sig_cld;
  int8_t sig_hup;
  int8_t sig_term;
  int8_t sig_quit;
  int8_t sig_urg;
  int8_t sig_usr1;
  int8_t sig_usr2;
  int16_t hellolen;		/* Length of hello message */
  int8_t down;			/* True if BBS down */
  int16_t limit;			/* User limit */
  int32_t cpuuser;			/* Save old value of user CPU */
  int32_t cpusys;			/* Save old value of sys CPU */
  pid_t pid;			/* Pid of running queue program */
  fd_set fds;
  time_t t;
  struct tm *ltm;
  time_t lastcheck;
  int32_t oldforks;
  int8_t lockout;
  int8_t init_info;
  int8_t init_reread;
  int16_t qindex[MAXQ];		/* Queue of fd's */

  struct btmp
  {
    int32_t eternal;
    uint32_t usernum;
    time_t time;
    int32_t remaddr;
    uint16_t remport;
    pid_t pid;
    char name[MAXALIAS + 1];
    char remote[40];
    char doing[40];
    char remlogin[16];
    int8_t xstat;
    int8_t elf;
    int8_t _was_anonymous;
    int8_t guest;
    int8_t connecting;
    int8_t client;
    int8_t nox;
    bool idle;
    int16_t sleeptimes;
    int32_t ulink;
  } btmp[MAXUSERS];

  struct qtmp
  {
    struct in_addr addr;// Peer addresses of fd's 
    time_t conn;		// Time connected 
    time_t last;		// Time last data arrived 
    int16_t qlo;			// Lowest point of fd in queue so far 
    int8_t acc;			// Count of useless keypresses 
    int8_t sgaloop;		// SGA loop counter 
    int8_t echoloop;		// ECHO loop counter 
    int8_t client;		// Is connected as a client 
    int8_t new;			// Flag set to create new user 
    int8_t checkup;		// Needs to check up on position 
    int8_t wasinq;		// Was in queue at some point 
    int8_t login;			// State counter for login 
    char name[20];		// BBS username 
    char pass[9];		// BBS passwd 
    int8_t options[64];
    int8_t do_dont_resp[64];
    int8_t will_wont_resp[64];
    char remoteusername[16];
    int8_t rows;
    int8_t initstate;
    int8_t state;
    int8_t ncc;
    uint8_t netibuf[32];
    uint8_t *netip;
    uint8_t netobuf[16];
    uint8_t *nfrontp;
    uint8_t *nbackp;
    uint8_t subbuffer[64];
    uint8_t *subpointer;
    uint8_t *subend;
    uint16_t port;
    int16_t wouldbe;
    uint8_t unused[4];
  } qt[MAXQ]; 
  /* unused? */
};

#define SEM_MSG		0
#define SEM_XMSG	1
#define SEM_USER	2
#define SEM_BTMP	3
#define SEM_NEWBIE	4
#define SEM_INDEX	5
#define SEM_VOTE	6


struct msg
{
  int32_t sem[7];
  int32_t    eternal;			/* New user number */
  int32_t    highest;			/* highest message number in file   */
  int32_t    curpos;			/* notate where the next msg will go */
  int32_t    xcurpos;
  int32_t    bcastpos;
  time_t  lastbcast;
  int16_t   maxusers;			/* Max users ever */
  int16_t   maxqueue;			/* Max queue ever */
  int16_t   maxtotal;			/* Max total ever */
  int16_t   unused1;
  int32_t    xmsgsize;			/* Size of X message data file */
  int16_t   maxnewbie;			/* Index of max newbie */
  int16_t   unused2;
  time_t  t;				/* Close to current time */
  int8_t    unused3[24];
  struct room
  {
    char    name[MAXNAME + 1];		/* Max. len is 39, plus null term   */
    uint32_t    roomaide;			/* User number of room aide         */
    int32_t    highest;			/* Highest message NUMBER in room   */
    int32_t    posted;			/* How many msgs posted in room     */
    char    passwd[11];			/* passworded room */
    int32_t    num[MSGSPERRM];		/* Universal message number   */
    int32_t    chron[MSGSPERRM];		/* chronological order in room of note*/
    int32_t    pos[MSGSPERRM];		/* Message positions in master file */
    int32_t    descupdate;			/* Eternal of last room desc update */
    uint8_t flags;                /* See flag values above            */
    int8_t    gen;			/* Generation number of room        */
    int8_t    unused[6];
  } room[MAXROOMS];
  struct newbie
  {
    char   name[20];
    time_t time;
  } newbies[MAXNEWBIES];
  int8_t shit[108];
};

#define M_MAGIC			0xfd

struct mheader
{
  uint8_t magic;
  uint32_t poster;
  bool deleted;
  uint32_t deleted_by_num;
  char deleted_by_name[MAXALIAS + 1];
  bool quotedx;
  bool mail;
  bool approval;
  uint16_t hlen;
  uint16_t len;
  uint32_t msgid;
  uint16_t forum;
  uint8_t mtype;
  time_t ptime;
  /* 128 bytes that can be used in the future to reduce 
  conversion updates */
  char poster_name[MAXALIAS + 1];
  char del_room_name[MAXNAME +1];
  uint8_t del_room_num;
  time_t dtime;
  uint8_t future_use[58];
  union
  {
    struct
    {
      uint32_t recipient;
    } mail;
  } ext;
};

#define X_NORMAL	0
#define X_QUESTION	1
#define X_BROADCAST	2

struct xheader
{
  uint32_t checkbit:1;
  uint32_t :6;
  uint32_t type:3;
  uint32_t snum;
  uint32_t rnum;
  uint32_t sprev;
  uint32_t snext;
  uint32_t rprev;
  uint32_t rnext;
  time_t time;
};


#define XPENDING	(msg->lastbcast > lastbcast || (ouruser && ouruser->xseenpos && (!mybtmp->nox || ouruser->f_xmsg)))
