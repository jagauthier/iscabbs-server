#include "defs.h"
#include "ext.h"

struct mheader_old {
  uint32_t magic : 8;
  uint32_t poster : 24;
  uint32_t : 5;
  uint32_t quotedx : 1;
  uint32_t mail : 1;
  uint32_t approval : 1;
  uint32_t hlen : 6;
  uint32_t len : 18;
  uint32_t msgid;
  uint16_t forum;
  uint8_t mtype;
  time_t ptime;

  union {
    struct {
      uint32_t : 8;
      uint32_t recipient : 24;
    } mail;
  } ext;
};

unsigned char *get_msgmain(char *);
struct userdata *get_users(char *);
struct user *find_user(struct userdata *, const char *, uint32_t);

void main(void) {
  unsigned char *msgstart_before, *msgstart_after;
  struct mheader *mh_after;
  struct mheader_old *mh_before;
  struct user *before, *after;
  struct userdata *udata_before, *udata_after;

  udata_before = get_users("/home/bbs/good/data/userdata");
  udata_after = get_users("/home/bbs/bad/data/userdata");

  msgstart_before = get_msgmain("/home/bbs/good/message/msgmain");
  msgstart_after = get_msgmain("/home/bbs/bad/message/msgmain");

  mh_after = (struct mheader *)msgstart_after;
  mh_before = (struct mheader_old *)msgstart_before;

  after = find_user(udata_after, "Dredd", 0);
  before = find_user(udata_before, "Dredd", 0);

  for (uint8_t m = 0; m < MAILMSGS; m++) {
    if (after->mr[m].num && after->mr[m].pos > 0) {
      if (before->mr[m].num != after->mr[m].num) {
        printf("That user before and after is not constistent at %i\n", m);
        continue;
      }
      mh_after = (struct mheader *)(msgstart_after + after->mr[m].pos);
      mh_before = (struct mheader_old *)(msgstart_before + before->mr[m].pos);
    }
  }
}