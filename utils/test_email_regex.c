#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <pcre.h>

#include "defs.h"
#include "ext.h"

extern char ansi;

int main() {
  // Read a block of text from the user

  ansi = 1;
  
  char *subject;
  
  struct mheader *mh;

  if (openfiles() < 0) {
    printf("Could not open BBS files.");
    return -1;
  }

  subject = calloc(1, 1000);
  strcpy(subject, "This is a mastadon username: @mastadon@domain.com. ");
  strcat(subject, "And this just an email address: mastadon@domain.com.");

  fix_mastadon(subject);

  printf("%s\n", subject);

  return 0;

  for (uint8_t r = 0; r < MAXROOMS; r++) {
    for (uint8_t p = 0; p < MSGSPERRM; p++) {
      if (!msg->room[r].pos[p]) {
        continue;
      }
      // colorize("Room %i, post: %i, pos: %i\n", r, p,msg->room[r].pos[p]);
      mh = (struct mheader *)(msgstart + msg->room[r].pos[p]);
      if (mh->magic != M_MAGIC) {
        printf("No magic.\n");
      } else {
        subject = calloc(mh->len * 2, 1);

        memcpy(subject, msgstart + msg->room[r].pos[p] + mh->hlen, mh->len);
        cleanup_strings(subject);

        colorize("@!%s@w\n", subject);
      }

      free(subject);
    }
  }

  return 0;
}
