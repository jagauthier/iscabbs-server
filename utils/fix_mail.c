#include "defs.h"
#include "ext.h"

struct mheader_old {
  uint8_t magic;
  uint32_t poster;
  bool deleted;
  uint32_t deleted_by;
  char deleted_name[MAXALIAS + 1];
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
  uint8_t future_use[108];
  union {
    struct {
      uint32_t recipient;
    } mail;
  } ext;
};

unsigned char *get_msgmain(char *);
struct userdata *get_users(char *);
struct user *find_user(struct userdata *, const char *, uint32_t);

void main(void) {
  unsigned char *msgstart_before, *msgstart_after;
  struct mheader *mh_after, *mail1, *mail2;
  struct mheader_old *mh_before;
  struct user *before, *after, *user1, *user2;
  struct userdata *udata_before, *udata_after;
  uint32_t mh_rcp_before, mh_rcp_after;
  uint8_t bfm_index = 0, afm_index = 0;
  uint32_t which;
  uint32_t totalusers;
  uint32_t link;
  uint32_t af_pos, bf_pos;

  udata_before = get_users("/home/bbs/before/data/userdata");
  udata_after = get_users("/home/bbs/data/userdata");

  msgstart_before = get_msgmain("/home/bbs/before/message/msgmain");
  msgstart_after = get_msgmain("/home/bbs/message/msgmain");

  /* Find the valid users. */
  which = udata_before->which;
  totalusers = udata_before->totalusers[which];

  for (uint16_t i = 0; i < totalusers; i++) {
    link = udata_before->num[which][i];
    before = (struct user *)(udata_before + 1) + (link);
    // before = find_user(udata_before, "Kierra", 0);
    after = find_user(udata_after, before->name, 0);

    if (!after) {
      printf("%s is gone?\n", before->name);
      continue;
    }

    printf("Fixing %s\n", before->name);
    for (afm_index = 0; afm_index < MAILMSGS; afm_index++) {
      /* Positive mail index is the message we've received .
        Negative is one that we've sent */
      af_pos = abs(after->mr[afm_index].pos);
      if (after->mr[afm_index].num) {
        // have to find the 'before' message because it's possible the user has
        // sent/recv mail
        for (bfm_index = 0; bfm_index < MAILMSGS; bfm_index++) {
          if (before->mr[bfm_index].num == after->mr[afm_index].num) {
            break;
          }
        }
        bf_pos = abs(before->mr[bfm_index].pos);
        mh_after = (struct mheader *)(msgstart_after + af_pos);
        mh_before = (struct mheader_old *)(msgstart_before + bf_pos);

        if (mh_before->magic != M_MAGIC) {
          printf("Before isn't magic.\n");
          exit(0);
        }
        if (mh_after->magic != M_MAGIC) {
          printf("After isn't magic.\n");
          exit(0);
        }

        if (mh_after->ext.mail.recipient != mh_before->ext.mail.recipient) {
          mh_rcp_before = (uint32_t)mh_before->ext.mail.recipient;
          mh_rcp_after = mh_after->ext.mail.recipient;
          printf("Recipient difference[%i, %i]: Current: %i Prior: %i\n",
                 afm_index, after->mr[afm_index].num, mh_rcp_after,
                 mh_rcp_before);
          mh_after->ext.mail.recipient = mh_before->ext.mail.recipient;
        }
      }
    }
    printf("Fixed %s\n", before->name);
  }
totalusers = udata_before->totalusers[which];
  /* Now that we've synced up old messages, let's look at syncing up current */
  for (uint16_t i = 0; i < totalusers; i++) {
    link = udata_after->num[which][i];
    user1 = (struct user *)(udata_after + 1) + (link);
    
        
    for (afm_index = 0; afm_index < MAILMSGS; afm_index++) {
      /* received message */
      if (user1->mr[afm_index].num) {
        mail1 =
            (struct mheader *)(msgstart_after + abs(user1->mr[afm_index].pos));
        if (!mail1->ext.mail.recipient) {
          /* Now find the other persons message */
          //printf("found a zero.\n");

          for (uint16_t j = 0; j < totalusers; j++) {
            link = udata_after->num[which][j];
            user2 = (struct user *)(udata_after + 1) + (link);
            if (user1->usernum == user2->usernum) {
              continue;
            }
            for (bfm_index = 0; bfm_index < MAILMSGS; bfm_index++) {
              if (user1->mr[afm_index].num == user2->mr[bfm_index].num) {
                printf("Found message %s/%s %i\n", user1->name, user2->name, mail1->msgid);
                if (user1->mr[afm_index].pos < 0 ) {
                    mail1->ext.mail.recipient = user2->usernum;
                } else {
                    mail1->ext.mail.recipient = user1->usernum;
                }
                break;
              }
            }
          }
        }
      }
    }
  }
}