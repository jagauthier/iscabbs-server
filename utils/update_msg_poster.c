#include "defs.h"
#include "ext.h"

void main(void) {
  struct room *room;
  struct mheader *mh;
  if (openfiles() < 0) {
    my_printf("openfiles() problem!\n");
    return;
  }

  for (uint8_t r = 0; r < MAXROOMS; r++) {
    room = &msg->room[r];
    if (room->posted) {
      printf("Room[%i]: %s\n", r, room->name);
      for (uint8_t p = 0; p < MSGSPERRM; p++) {
        mh = (struct mheader *)(msgstart + room->pos[p]);
        if (mh->magic != M_MAGIC) {
          printf("There is no magic: %i %i\n", p, room->pos[p]);
        }
        if (!mh->deleted) {
          char *name = getusername(mh->poster, 1);
          /* deleted user */
            strcpy(mh->poster_name, name);
        }
      }
    }
  }
}