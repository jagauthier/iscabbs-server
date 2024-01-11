/*
 * who.c - Handles display of profiles and who list for BBS and finger daemon.
 */
#include "defs.h"
#include "ext.h"

/*
 * show_online()
 * Show a list of the users online, levels of detail:
 *   0 regular list
 *   1 wizard's debugging info
 *   2 TCP finger port list
 *   3 short list
 */

void show_online(int8_t level) {
  struct btmp *btmp;
  int16_t l;
  char msg_status;
  int32_t i;
  int32_t tdif;
  uint16_t min, hour, pos;
  char work[24];
  time_t curr_time;
  int32_t mineternal;
  int16_t done = 0;
  int32_t whostart, whoend, whoincr;

  uint16_t connecting_users = 0;

  if (!bigbtmp->users) {
    return;
  }

  if (level == 0) {
    for (uint16_t i = 0; i < bigbtmp->users; i++) {
      btmp = &bigbtmp->btmp[bigbtmp->index[i]];
      if (btmp->connecting) {
        connecting_users++;
      }
    }
  }

  if (level != 2 && rows != 32000) {
    my_putchar('\n');
  }
  if (bigbtmp->users > 1) {
    colorize("There are "BOLD_YELLOW"%i"BOLD_GREEN" users ("BOLD_YELLOW"%i"BOLD_GREEN" queued)",
             (bigbtmp->users - connecting_users), bigbtmp->queued);
  } else {
    colorize("There is "BOLD_YELLOW"1"BOLD_GREEN" user ("BOLD_YELLOW"%i"BOLD_GREEN" queued)", bigbtmp->queued);
  }

  my_printf("\n\n");

  curr_time = time(0);
  if (level == 1) {
    colorize(BOLD_YELLOW"User Name             "BOLD_MAGENTA"PID  "BOLD_MAGENTA"eternal "BOLD_RED"Time  "BOLD_CYAN"From\n"BOLD_GREEN);
  } else if (level == 4) {
    colorize(BOLD_YELLOW"Doing list\n");
  } else if (level != 3) {
    colorize(BOLD_YELLOW"User Name                "BOLD_MAGENTA"Time "BOLD_RED"Doing           "BOLD_WHITE"%14s\n"BOLD_GREEN,
             formtime(3, curr_time));
  }

  if (level != 3) {
    my_printf("------------------------------------------------------------\n");
  }

  if (level == 3) {
    l = 2;
  } else {
    l = 4;
  }
  pos = 0;

  whostart = bigbtmp->users - 1;
  whoend = 0;
  whoincr = -1;

  if ((ouruser != NULL) && ouruser->f_revwho) {
    whoincr = 1;
    whoend = whostart;
    whostart = 0;
  }

  for (mineternal = ((whoincr < 0) ? 999999999 : 0), i = whostart;
       (whoincr < 0) ? (i >= whoend) : (i <= whoend); i += whoincr) {
    btmp = &bigbtmp->btmp[bigbtmp->index[i]];
    if (!btmp->pid || ((whoincr < 0) ? (btmp->eternal >= mineternal)
                                     : (btmp->eternal <= mineternal))) {
      continue;
    }

    if (level == 0 && btmp->connecting) {
      continue;
    }

    mineternal = btmp->eternal;
    done++;

    tdif = curr_time - btmp->time;
    tdif /= 60;
    min = tdif % 60;
    hour = tdif / 60;

    msg_status = btmp->xstat ? '*' : SP;

    switch (level) {
      case 4: {
        colorize(BOLD_YELLOW"%s is %s\n", btmp->name, btmp->doing);
        break;
      }

      case 1: {
        char work2[60];

        checked_snprintf(work2, sizeof(work2), "%s%s%s", btmp->remlogin,
                         *btmp->remlogin ? "@@" : "", btmp->remote);
        colorize(BOLD_YELLOW"%-19s "BOLD_MAGENTA"%5d "BOLD_MAGENTA"%3i "BOLD_RED"%2d:%02d%c "BOLD_CYAN"%.37s\n", btmp->name,
                 btmp->pid, btmp->eternal, hour, min, btmp->client ? 'C' : ' ',
                 work2);
      } break;
      case 3:
        if ((msg_status =
                 ' '))  // TODO: This makes no sense. It's probably a bug.
          msg_status = ' ';

        if (btmp->elf && !btmp->xstat) {
          msg_status = '%';
        }

        if (pos == 3) {
          checked_snprintf(work, sizeof(work), "%c%s", msg_status, btmp->name);
        } else {
          checked_snprintf(work, sizeof(work), "%c%-19s", msg_status,
                           btmp->name);
        }
        break;
      case 0:
      case 2:
      default: {
        if (btmp->elf && !btmp->xstat) {
          msg_status = '%';
        }
        if (btmp->sleeptimes > 5) {
          msg_status = '#';
        }
        colorize(BOLD_YELLOW"%-19s %c "BOLD_RED"%4d:%02d ", btmp->name, msg_status, hour, min);

        colorize(""BOLD_MAGENTA"%-41s\n", btmp->doing);

        break;
      }
    }
    if (level == 3) {
      if (*work) {
        fputs(work, stdout);
      } else {
        pos--;
      }
      if (++pos == 4) {
        pos = 0;
        my_putchar('\n');
      } else {
        continue;
      }
    }
    if (++l >= rows - 1 &&
        line_more(&l, (done * 100) / (bigbtmp->users + 1)) < 0) {
      break;
    }
  }
  if (level == 3 && pos) {
    my_putchar('\n');
  }
}

/*
 * is_online (username)
 *
 * Looks at the btmp file and returns true if the user is currently online.
 */

struct btmp *is_online(struct btmp *btmp, struct user *user, const char *name) {
  int16_t i;

  if (user) {
    if ((i = user->btmpindex) >= 0) {
      if (bigbtmp->btmp[i].pid && bigbtmp->btmp[i].usernum == user->usernum) {
        if (btmp) {
          *btmp = bigbtmp->btmp[i];
        }
        return (&bigbtmp->btmp[i]);
      } else {
        locks(SEM_USER);
        if (user->btmpindex >= 0 &&
            (!bigbtmp->btmp[i].pid ||
             bigbtmp->btmp[i].usernum != user->usernum)) {
          user->btmpindex = -1;
        }
        unlocks(SEM_USER);
      }
    }
    return (NULL);
  }

  for (i = 0; i < MAXUSERS; i++)
    if (bigbtmp->btmp[i].pid && !strcmp(bigbtmp->btmp[i].name, name)) {
      if (btmp) {
        *btmp = bigbtmp->btmp[i];
      }
      return (&bigbtmp->btmp[i]);
    }
  return (NULL);
}

/*
 * Print out the profile of the user listed.
 *
 * If the name is blank, do yourself. If the user is an aide show everything.
 */

int profile(const char *name, struct user *tuser, int flags) {
  struct user *tmpuser;
  struct btmp userstat;
  bool online;
  bool showanon;
  
  if (!name) {
    tmpuser = tuser;
  } else if (*name) {
    if (!(tmpuser = getuser(name)) ||
        (tmpuser->f_invisible && flags != PROF_ALL)) {
      if (tmpuser) {
        freeuser(tmpuser);
      }
      return (-1);
    }
  } else
    tmpuser = ouruser;

  if (rows != 32000) {
    colorize(BOLD_CYAN"\n");
  }

  my_printf("%s", tmpuser->name);

  if (tmpuser->f_prog && flags == PROF_ALL) {
    colorize(BOLD_RED" *Programmer*");
  }
  if (tmpuser->f_aide) {
    colorize(BOLD_RED" *Sysop*");
  }

  if (tmpuser->f_elf && flags == PROF_ALL) {
    colorize(BOLD_WHITE" %%guide%%");
  }

  if (tmpuser->f_twit) {
    colorize(BOLD_WHITE" -TWIT-");
  }
  if (tmpuser->f_newbie) {
    colorize(BOLD_RED" (new)");
  }
  if (flags == PROF_ALL) {
    if (tmpuser->f_deleted) {
      colorize(BOLD_RED" (deleted)");
    }
    if (tmpuser->f_inactive) {
      colorize(BOLD_RED" (inactive)");
    }
    if (tmpuser->f_badinfo) {
      colorize(BOLD_RED" (bad info)");
    }
    if (tmpuser->f_duplicate) {
      colorize(BOLD_RED" (duplicate)");
    }
    if (tmpuser->f_trouble) {
      colorize(BOLD_RED" (trouble?)");
    }
  }
  colorize(BOLD_YELLOW"\n");
  if (*tmpuser->vanityflag) {
    colorize(BOLD_GREEN"%s\n", tmpuser->vanityflag);
  }

  showanon = (flags == PROF_ALL || flags == PROF_SELF);
  if (showanon || !tmpuser->an_all) {
    if (*tmpuser->real_name &&
        (showanon || (!tmpuser->an_all && !tmpuser->an_name))) {
      if (tmpuser->an_all || tmpuser->an_name) {
        colorize(BOLD_WHITE"HIDDEN> "BOLD_YELLOW);
      }
      my_printf("%s\n", tmpuser->real_name);
    }
    if (*tmpuser->addr1 &&
        (showanon || (!tmpuser->an_all && !tmpuser->an_addr))) {
      if (tmpuser->an_all || tmpuser->an_addr) {
        colorize(BOLD_WHITE"HIDDEN> "BOLD_YELLOW);
      }
      my_printf("%s\n", tmpuser->addr1);
    }
    if (*tmpuser->addr2) my_printf("%s\n", tmpuser->addr2);

    if (*tmpuser->city &&
        (showanon || (!tmpuser->an_all && !tmpuser->an_location))) {
      if (tmpuser->an_all || tmpuser->an_location) {
        colorize(BOLD_WHITE"HIDDEN> "BOLD_YELLOW);
      }
      my_printf("%s, %s  %s\n", tmpuser->city, tmpuser->state, tmpuser->zip);
    }
    if (*tmpuser->phone &&
        (showanon || (!tmpuser->an_all && !tmpuser->an_phone))) {
      if (tmpuser->an_all || tmpuser->an_phone) {
        colorize(BOLD_WHITE"HIDDEN> ");
      }
      colorize(BOLD_GREEN"Phone: "BOLD_YELLOW);
      my_printf("%s\n", tmpuser->phone);
    }
    if (*tmpuser->mail &&
        (showanon || (!tmpuser->an_all && !tmpuser->an_mail))) {
      if (tmpuser->an_all || tmpuser->an_mail) {
        colorize(BOLD_WHITE"HIDDEN> ");
      }
      colorize(BOLD_GREEN"Email: "BOLD_YELLOW);
      my_printf("%s\n", tmpuser->mail);
    }
    if (*tmpuser->www && (showanon || (!tmpuser->an_all && !tmpuser->an_www))) {
      if (tmpuser->an_all || tmpuser->an_www) {
        colorize(BOLD_WHITE"HIDDEN> ");
      }
      colorize(BOLD_GREEN"WWW: "BOLD_YELLOW);
      my_printf("%s\n", tmpuser->www);
    }
  }

  online = is_online(&userstat, tmpuser, NULL) ? TRUE : FALSE;
  if (tmpuser->time) {
    if (online) {
      colorize(BOLD_RED"ONLINE since:");
    } else {
      colorize(BOLD_GREEN"Last on:");
    }
    colorize(BOLD_MAGENTA" %s", formtime(3, tmpuser->time));
    if (online || tmpuser->timeoff < tmpuser->time) {
      if (!showanon && tmpuser->an_site) {
        my_putchar('\n');
      } else {
        colorize("\n");
      }
    } else {
      if (!showanon && tmpuser->an_site) {
        colorize(" until %s\n", formtime(4, tmpuser->timeoff));
      } else {
        colorize(" until %s "BOLD_GREEN"from "BOLD_MAGENTA"%.38s\n", formtime(4, tmpuser->timeoff),
                 tmpuser->remote);
      }
    }

    colorize(
        BOLD_GREEN"Times called:"BOLD_MAGENTA" %d "BOLD_GREEN"Messages posted:"BOLD_MAGENTA" %d "BOLD_GREEN"X messages sent:"BOLD_MAGENTA" "
        "%i "
        BOLD_GREEN"User# "BOLD_MAGENTA"%i\n"BOLD_GREEN,
        tmpuser->timescalled, tmpuser->posted, tmpuser->totalx,
        tmpuser->usernum);

    if (flags == PROF_ALL) {
      if (tmpuser->f_aide) {
        colorize(BOLD_GREEN"Yells handled:"BOLD_MAGENTA" %d "BOLD_GREEN"Validations done:"BOLD_MAGENTA" %d\n"BOLD_GREEN,
                 tmpuser->yells, tmpuser->vals);
      }

      colorize(BOLD_GREEN"created: "BOLD_MAGENTA"%s   ", formtime(3, tmpuser->firstcall));

      if (online) {
        colorize(BOLD_GREEN"pid: "BOLD_YELLOW"%d  ", userstat.pid);
      }
      if (*tmpuser->loginname) {
        colorize(BOLD_GREEN"loginname: "BOLD_YELLOW"%s", tmpuser->loginname);
      }
      if (online && userstat.client) {
        colorize(BOLD_WHITE"  (client)");
      }
      if (*tmpuser->remote) {
        colorize("\n"BOLD_GREEN"hostname: "BOLD_YELLOW"%s", tmpuser->remote);
      }
      my_putchar('\n');
    }
    /*
        else if (flags != PROF_REG)
          colorize(BOLD_GREEN"User# "BOLD_MAGENTA"%ld\n", tmpuser->usernum);
    */

    if (*tmpuser->doing) {
      colorize(BOLD_GREEN"Doing: "BOLD_CYAN"%-41s", tmpuser->doing);
    }

    if (online && userstat.sleeptimes) {
      colorize(BOLD_GREEN"[idle for "BOLD_YELLOW"%02d:%02d"BOLD_GREEN"]", userstat.sleeptimes / 60,
               userstat.sleeptimes % 60);
    }
    /* If we just printed the above, we need a newline */
    if (*tmpuser->doing || (online && userstat.sleeptimes)) {
      my_printf("\n");
    }

    if (online && rows != 32000) {
      if (userstat.xstat) {
        colorize(BOLD_RED"[eXpress messages DISABLED]\n");
      } else if (userstat.elf) {
        colorize(
            BOLD_RED"[This user can be eXpressed if you need help with the "
            "system]\n");
      }
    }
  }

  colorize(BOLD_GREEN);

  if (flags == PROF_ALL && *tmpuser->aideinfo) {
    my_printf("%s\n", tmpuser->aideinfo);
  }

  if (*tmpuser->desc1) {
    colorize("\n%s\n", tmpuser->desc1);
  }
  if (*tmpuser->desc2) {
    colorize("%s\n", tmpuser->desc2);
  }
  if (*tmpuser->desc3) {
    colorize("%s\n", tmpuser->desc3);
  }
  if (*tmpuser->desc4) {
    colorize("%s\n", tmpuser->desc4);
  }
  if (*tmpuser->desc5) {
    colorize("%s\n", tmpuser->desc5);
  }

  if (!tuser) {
    freeuser(tmpuser);
  }
  return (0);
}
