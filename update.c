/*
 * Deletes deleteable user records, updates sysop and who knows forum lists.
 */
#include "defs.h"
#include "ext.h"

#define LIMIT 26
#undef MARGIN
#define MARGIN 54

#ifdef _UPDATEHACK
static int addxconf(struct user *u, int32_t num, struct xconf x[60], int which);
static int hack_xconf(struct user *u);
#endif

struct userinfo {
  uint32_t usernum;
  char name[MAXNAME + 1];
  char generation[MAXROOMS];
  char forget[MAXROOMS];
  uint32_t f_admin : 1;
  uint32_t f_prog : 1;
  uint32_t f_aide : 1;
};

static struct userinfo *getusers(uint32_t *ucount);
static int update_aides(struct userinfo *start, uint32_t *ucount);
static int update_whoknows(struct userinfo *start, uint32_t *ucount);

void bbsupdate(void) {
  struct userinfo *start;
  uint32_t ucount;

  if (!(start = getusers(&ucount))) {
    my_printf("Error reading user files\n");
    fflush(stdout);
    _exit(1);
  }

#ifdef _UPDATEHACK
  fflush(stdout);
  _exit(0);
#endif

  if (update_aides(start, &ucount)) {
    my_printf("Error updating aide list\n");
    fflush(stdout);
    _exit(1);
  }

  if (update_whoknows(start, &ucount)) {
    my_printf("Error updating whoknowsroom lists\n");
    fflush(stdout);
    _exit(1);
  }

  fflush(stdout);
}

static struct userinfo *getusers(uint32_t *ucount) {
  int i;
  struct userinfo *start;
  struct user *up;
  struct userinfo *uinfo;
  struct userdata *ucopy;
  uint32_t unbr = 0;
  struct tm *ltm;
  FILE *ulog;
  bool zap;
  int32_t days_since_on;
  uint32_t d;
  int16_t st_tot = 0;
  int16_t st_lold = 0;
  int16_t st_npro = 0;
  int16_t st_nold = 0;
  int16_t st_nov = 0;
  int16_t st_ansi = 0;
  FILE *info;
  time_t t;

  if (!(ucopy = copyuserdata())) {
    my_printf("can't copy userdata\n");
    return (NULL);
  }

  *ucount = ucopy->totalusers[ucopy->which];

  uinfo = (struct userinfo *)mmap(0, *ucount * sizeof(struct userinfo),
                                  PROT_READ | PROT_WRITE,
                                  MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  if (!uinfo || uinfo == (struct userinfo *)-1) {
    my_printf("bad uinfo\n");
    return (NULL);
  }
  start = uinfo;

  if (!(ulog = fopen(ROOT "var/userlist", "w"))) {
    perror("open userlist");
    return (NULL);
  }
  fprintf(ulog,
          "   date    time   call   post    x       priv  name                 "
          "realname               email                                     "
          "connection\n");

  if (!(info = fopen(ROOT "var/info", "w"))) {
    perror("can't open info");
    return (NULL);
  }

  t = time(0);
  for (d = 0; d < *ucount; d++) {
    zap = TRUE;
    if ((up = finduser(NULL, ucopy->link[ucopy->name[ucopy->which][d]].usernum,
                       0))) {
      zap = FALSE;
      days_since_on = (t - up->time) / 86400;

      /* Zap if new user and hasn't called in 30 days */
      if (days_since_on > 30 && up->f_newbie) {
        zap = TRUE;
      }
      /* Zap if not called in 2 months and called less than 10 x */
      else if (days_since_on > 60 && up->timescalled < 10) {
        zap = TRUE;
      }
      /* Zap if user hasn't called in 2 years */
      else if (days_since_on > 730) {
        zap = TRUE;
      }
      /* Zap if we have marked for deletion */
      else if (up->f_deleted && days_since_on >= 1) {
        zap = TRUE;
      }
      /* Don't delete Guest */
      if (!strcmp(up->name, "Guest")) {
        zap = FALSE;
      }
    } else {
      fprintf(info, "Didn't find user '%s'\n",
              ucopy->link[ucopy->name[ucopy->which][d]].name);
      continue;
    }

    if (zap) {
      deleteuser(up->name);
    } else {
      uinfo[unbr].usernum = up->usernum;
      strcpy(uinfo[unbr].name, up->name);
      uinfo[unbr].f_admin = up->f_admin;
      uinfo[unbr].f_prog = up->f_prog;
      uinfo[unbr].f_aide = up->f_aide;
      memcpy(uinfo[unbr].generation, (const char *)(up->generation), MAXROOMS);
      memcpy(uinfo[unbr].forget, (const char *)(up->forget), MAXROOMS);
#ifdef _UPDATEHACK
      hack_xconf(up);
#endif
      unbr++;

      st_tot++;
      if (up->f_ansi) {
        st_ansi++;
      }

      ltm = localtime(&up->time);
      i = strcmp(up->A_real_name, up->real_name) ||
          strcmp(up->A_addr1, up->addr1) || strcmp(up->A_city, up->city) ||
          strcmp(up->A_state, up->state) || strcmp(up->A_zip, up->zip) ||
          strcmp(up->A_phone, up->phone) || strcmp(up->A_mail, up->mail);
      fprintf(ulog,
              "%s%02d%02d%02d  %02d:%02d  %5d  %6d  %6i  0000  %-19s: %-20s : "
              "%-40s; %s@%s\n",
              i ? "** " : "   ", ltm->tm_year, ltm->tm_mon + 1, ltm->tm_mday,
              ltm->tm_hour, ltm->tm_min, up->timescalled, up->posted,
              up->totalx, up->name, up->real_name, up->mail, up->loginname,
              up->remote);
    }
  }
  fprintf(
      info,
      "\n\nlold = %d, npro = %d, nold = %d, nov = %d, ansi = %d, tot = %d\n",
      st_lold, st_npro, st_nold, st_nov, st_ansi, st_tot);
  fclose(info);
  fclose(ulog);
  *ucount = unbr;
  return (start);
}

// The following are a couple comparison functions needed for qsort()
// calls in update_aides.

int cmp_userinfo(                   // return lt, eq, gt for userinfo structs
    const struct userinfo *first,   // based on their usernumber.  see
    const struct userinfo *second)  // man 3 qsort for details
{
  if (first->usernum > second->usernum) {
    return (1);
  } else if (first->usernum < second->usernum) {
    return (-1);
  } else {
    return (0);
  }
}

int cmp_room_glob(                   // return lt, eq, gt for room_glob structs
    const struct room_glob *first,   // based on alphabetical of the roomname
    const struct room_glob *second)  // case-insensitive
{
  int8_t c = 0, retval = 0;

  // cheap hack for comparison in 'alphabetical order'

  do {
    if (toupper(first->roomname[c]) < toupper(second->roomname[c])) {
      retval--;
    } else if (toupper(first->roomname[c]) > toupper(second->roomname[c])) {
      retval++;
    }

    c++;
  } while (!retval &&
           !(first->roomname[c] == '\0' || first->roomname[c] == ' ') &&
           !(second->roomname[c] == '\0' || second->roomname[c] == ' '));

  return retval;

  // return(strcasecmp(first->roomname, second->roomname));
}

// End comparison functions needed for qsort() calls in update_aides.

static int update_aides(struct userinfo *start, uint32_t *ucount) {
  FILE *af;
  // int     nameflag = NO;
  uint8_t room_number;
  struct userinfo *curruser, *bkey, dummy;
  uint32_t user_number;

  struct room_glob roomdata[MAXROOMS];

  uint16_t max_display_rooms = 0;  // max # of rooms we'll display
  uint8_t c;

  size_t num_elements, size;

  bkey = &dummy;

  const char *const name = AIDELIST ".NEW";
  if (!(af = fopen(name, "w"))) {
    return (-1);
  }

  fprintf(af, "Sysops and Programmers\n");

  curruser = start;

  // below, we loop through all users, and display their names and user
  // numbers if they're a programmer or sysop.  Eventually we'll do
  // them in alphabetical order and possibly tabbed output, but for
  // now, it's quick and dirty line by line, by user number.

  for (user_number = 0; user_number < *ucount; user_number++) {
    if (curruser->f_admin) {
      fprintf(af, "\n %s(%i) %s%s", curruser->name, curruser->usernum,
              (curruser->f_prog ? "*Programmer* " : ""),
              (curruser->f_aide ? "*Sysop*" : ""));
    }
    curruser++;  // increment pointer
  }

  fprintf(af, "\n\n");

  // Now that we've displayed the Sysops and Programmers at the top
  // of the file, we need to populate the roomdata array with the
  // names and numbers of rooms, and the usernumbers of their forum
  // moderators (the names of their moderators will be taken care
  // of later)

  c = 0;  // use this as roomdata[] index since we're skipping pvt. rooms
          // remember... read from room_number, write to c   :-P

  for (room_number = 0; room_number < MAXROOMS; room_number++) {
    if ((msg->room[room_number].flags & QR_INUSE) &&
        !(msg->room[room_number].flags & QR_PRIVATE) &&
        room_number != MAIL_RM_NBR) {
      *roomdata[c].roomname = 0;  // set 1st char to null for str funcs
      strcat(roomdata[c].roomname, msg->room[room_number].name);

      roomdata[c].roomnumber = room_number;
      roomdata[c].moderator_number = msg->room[room_number].roomaide;

      max_display_rooms++;  // use this later, since private rooms are not
                            // included, we can't just use # of active rms
      c++;                  // increment roomdata[] index
    }
  }

  // Now we fill in the Roomaide names that aren't in the msg struct...
  // Since getusers() returns the array in alphabetical order by username,
  // we need to do a qsort on the array to get it into numeric order.  I
  // would have just modified getusers(), but it's hairy and I'm not that
  // good yet, so we pay a worst-case O(n*n) penalty for the qsort (plus
  // the function call penalty, since I'm using the stdlib qsort... so
  // eventually getusers() should be rewritten to return the array in
  // numeric order by usernumber.  That way we don't need to pay a penalty
  // resorting the array of structs, and can still use the binary search
  // by usernumber.

  // setup stuff for qsort()
  num_elements = *ucount;
  size = sizeof(*start);

  // sort array of userinfo structs into ascending numeric order by usernum
  qsort(start, num_elements, size,
        (int (*)(const void *, const void *))cmp_userinfo);

  // now we do a binary search to find the user name associated with the
  // room's moderator usernumber.  If the moderator usernumber is zero,
  // that means it's set to generic 'Sysop'
  for (c = 0; c < max_display_rooms; c++) {
    *roomdata[c].moderator_name = 0;

    curruser = start;
    curruser = start;  // assign the pointer to the start of user array
    if (roomdata[c].moderator_number) {
      bkey->usernum = roomdata[c].moderator_number;
      curruser = bsearch(bkey, start, num_elements, size,
                         (int (*)(const void *, const void *))cmp_userinfo);

      /*if (curruser->usernum == roomdata[c].moderator_number)*/
      if (curruser && curruser->usernum == roomdata[c].moderator_number) {
        strcat(roomdata[c].moderator_name, curruser->name);
      } else {
        // we shouldn't ever get here now that code has been debugged
        checked_snprintf(roomdata[c].moderator_name,
                         sizeof(roomdata[c].moderator_name), "User %i",
                         roomdata[c].moderator_number);
      }
    } else {
      strcat(roomdata[c].moderator_name, "(Sysop)");
    }
  }

  // Now we want to sort the array of filled-in room_glob structs into
  // alphabetical order by roomname.

  num_elements = max_display_rooms;
  size = sizeof(*roomdata);

  qsort(roomdata, num_elements, size,
        (int (*)(const void *, const void *))cmp_room_glob);

  // Now we display the array

  fprintf(af, "%s", "Forum Moderators\n\n");

  for (c = 0; c < max_display_rooms; c++) {
    int len = fprintf(af, " \"%s>\"", roomdata[c].roomname);

    while (len < (MAXNAME + 6)) {
      len += fprintf(af, " ");
    }

    fprintf(af, " \"%s\"\n", roomdata[c].moderator_name);
  }

  {
    struct tm *fun;
    time_t *fun2, buff;

    buff = time(NULL);
    fun2 = &buff;
    fun = localtime(fun2);

    fprintf(af, "\nLast Updated: %d-%d-%d %d:%02d", fun->tm_mon + 1,
            fun->tm_mday, fun->tm_year + 1900, fun->tm_hour, fun->tm_min);
  }

  (void)fclose(af);
  rename(name, AIDELIST);
  return (0);
}

static int update_whoknows(struct userinfo *start, uint32_t *ucount) {
  FILE *file;
  char filestr[160];
  char name[100];
  char newname[100];
  uint8_t rm_nbr;
  struct userinfo *u;
  uint32_t unbr;

  for (rm_nbr = 0; rm_nbr < MAXROOMS; rm_nbr++) {
    u = start;

    checked_snprintf(name, sizeof(name), "%srm%d.NEW", WHODIR, rm_nbr);
    checked_snprintf(newname, sizeof(newname), "%srm%d", WHODIR, rm_nbr);

    if (!(file = fopen(name, "w"))) {
      return (-1);
    }

    if (!(msg->room[rm_nbr].flags & QR_INUSE)) {
      fclose(file);
      rename(name, newname);
      continue;
    }

    /* make a heading in the whoknows file for this room */
    checked_snprintf(filestr, sizeof(filestr), "\nWho knows \"%s\"\n\n",
                     msg->room[rm_nbr].name);

    if (rm_nbr < 2) {
      fprintf(file, "%sEVERYONE knows this forum.\n", filestr);
      fclose(file);
      rename(name, newname);
      continue;
    }

    fprintf(file, "%s", filestr);
    *filestr = 0;

    for (unbr = 0; unbr < *ucount; unbr++) {
      if (msg->room[rm_nbr].gen != u->forget[rm_nbr] &&
          u->generation[rm_nbr] != RODSERLING &&
          ((!(msg->room[rm_nbr].flags & QR_PRIVATE) &&
            u->forget[rm_nbr] != NEWUSERFORGET) ||
           msg->room[rm_nbr].gen == u->generation[rm_nbr])) {
        char temp[sizeof(filestr)];
        checked_snprintf(temp, sizeof(temp), "%s (%i)", u->name, u->usernum);
        // This appears to be trying to create two columns, by alternating
        // every other iteration.
        if (!*filestr) {
          checked_strcat(filestr, sizeof(filestr), temp);
        } else {
          checked_strcat(filestr, sizeof(filestr),
                         "                             ");
          checked_strcat(filestr, sizeof(filestr), temp);
          fprintf(file, "%s\n", filestr);
          *filestr = 0;
        }
      }
      u++;
    } /* end users for loop */
    if (*filestr) {
      fprintf(file, "%s\n", filestr);
    }
    fclose(file);
    rename(name, newname);
  } /* end rooms for loop */
  return (0);
}

#ifdef _UPDATEHACK
struct xconf {
  unsigned int : 7;
  unsigned int which : 1;    /* 0 = disable, 1 = enable   */
  unsigned int usernum : 24; /* Username to enable/disable*/
};

static int hack_xconf(struct user *u) {
  int i;
  struct xconf x[60];

  if ((sizeof x) != 240) {
    my_printf("Sizeof x is %d\n", sizeof x);
    return 0;
  }
  bzero(x, sizeof x);
  for (i = 0; i < 10; i++) {
    if (u->xno[i].usernum) addxconf(u, u->xno[i].usernum, x, 0);
    if (u->xyes[i].usernum) addxconf(u, u->xyes[i].usernum, x, 1);
  }
#if 0
  bzero((void *)u->xno, sizeof u->xno);
  bcopy(x, (struct xconf *)(void *)u->xyes, sizeof x);
#endif
  return 0;
}

static int addxconf(struct user *u, long num, struct xconf x[60], int which) {
  char *name, *tmpname;
  int i, j;

  name = getusername(num, 0);
  if (!name) return 0;
  for (i = 0; i < 20 && x[i].usernum; i++) {
    tmpname = getusername(x[i].usernum, 0);
    if (!tmpname) {
      my_printf("Can't happen here\n");
      return 0;
    }
    if (!strcmp(name, tmpname)) return 0;
    if (strcmp(name, tmpname) < 0) break;
  }
  if (i == 20) {
    my_printf("List full, can't happen\n");
    return 0;
  }
  for (j = 19; j > i; j--) x[j] = x[j - 1];
  x[i].usernum = num;
  x[i].which = which;
  return 0;
}
#endif
