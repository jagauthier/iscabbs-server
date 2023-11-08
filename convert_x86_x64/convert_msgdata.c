#include "convert_utils.h"
#include <locale.h>

uint8_t convert_msgdata(void) {
  FILE *fp;
  struct msg temp_msg;
  uint8_t *mcfp = NULL;

  /* Open the file directly and then read through it generating a new
  bigbtmp with new data sizes.  This should work on x86 and x64 */
  fp = fopen("/home/bbs/data.orig/msgdata", "r");
  if (!fp) {
    printf("Could not open /home/bbs/data.orig/msgdata\n");
    return 1;
  }

  fseek(fp, 0, SEEK_END);
  printf("msgdata file size: %li\n", ftell(fp));
  fseek(fp, 0, SEEK_SET);

  memset(&temp_msg, 0, sizeof(struct msg));

  fread(&temp_msg.sem, 7, S32, fp);
  fread(&temp_msg.eternal, 1, S32, fp);
  fread(&temp_msg.highest, 1, S32, fp);
  fread(&temp_msg.curpos, 1, S32, fp);
  printf("Original next post location: %#x\n", temp_msg.curpos);
  if (!x86) {
    int32_t new_pos = find_magic_location(temp_msg.curpos);
    temp_msg.curpos += new_pos * 4;
    printf("New next post location: %#x\n", temp_msg.curpos);
  }
  fread(&temp_msg.xcurpos, 1, S32, fp);
  fread(&temp_msg.bcastpos, 1, S32, fp);
  fread(&temp_msg.lastbcast, 1, S32, fp);
  fread(&temp_msg.maxusers, 1, S16, fp);
  fread(&temp_msg.maxqueue, 1, S16, fp);
  fread(&temp_msg.maxtotal, 1, S16, fp);
  fread(&temp_msg.unused1, 1, S16, fp);
  fread(&temp_msg.xmsgsize, 1, S32, fp);
  temp_msg.xmsgsize = XMSGSIZE;
  setlocale(LC_ALL, "");
  printf("xmsgsize set to %'i\n", temp_msg.xmsgsize);
  fread(&temp_msg.maxnewbie, 1, S16, fp);
  fread(&temp_msg.unused2, 1, S16, fp);
  fread(&temp_msg.t, 1, S32, fp);
  fread(&temp_msg.unused3, 24, S8, fp);

  for (uint16_t i = 0; i < MAXROOMS; i++) {
    fread(&temp_msg.room[i].name, MAXNAME + 1, S8, fp);
    // printf("Converting room: %s\n", temp_msg.room[i].name);
    //  move forward
    fseek(fp, 3, SEEK_CUR);
    fread(&temp_msg.room[i].roomaide, 1, S32, fp);
    fread(&temp_msg.room[i].highest, 1, S32, fp);
    fread(&temp_msg.room[i].posted, 1, S32, fp);
    fread(&temp_msg.room[i].passwd, 11, S8, fp);
    // 11 bytes boundaries up to 12 in the file
    fseek(fp, 1, SEEK_CUR);
    fread(&temp_msg.room[i].num, MSGSPERRM, S32, fp);
    fread(&temp_msg.room[i].chron, MSGSPERRM, S32, fp);
    fread(&temp_msg.room[i].pos, MSGSPERRM, S32, fp);
    fread(&temp_msg.room[i].descupdate, 1, S32, fp);
    fread(&temp_msg.room[i].flags, 1, S8, fp);
    fread(&temp_msg.room[i].gen, 1, S8, fp);
    fread(&temp_msg.room[i].unused, 6, S8, fp);
  }
  for (uint16_t i = 0; i < MAXNEWBIES; i++) {
    fread(&temp_msg.newbies[i].name, 20, S8, fp);
    fread(&temp_msg.newbies[i].time, 1, S32, fp);
  }
  fseek(fp, 108, SEEK_CUR);
  fclose(fp);

  fp = fopen("convert_output/data/msgdata", "w");
  fwrite(&temp_msg, sizeof(struct msg), 1, fp);
  printf("msgdata: Expected: %u, Actual: %li\n\n", (uint32_t)sizeof(struct msg),
         ftell(fp));
  fclose(fp);
  if (mcfp) {
    free(mcfp);
  }
  return 0;
}
