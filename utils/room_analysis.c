#include "defs.h"
#include "ext.h"

void main(void) {
    struct room *room; 
    struct user *kena;
    if (openfiles() < 0) {
        my_printf("openfiles() problem!\n");
        return (-1);
    }

   room = &msg->room[DELMSG_RM_NBR];
   kena = finduser("Kena", 0, 0);
   printf("Room: %i\n", room->highest);
   printf("Kena: %i\n", kena->lastseen[DELMSG_RM_NBR]);

}