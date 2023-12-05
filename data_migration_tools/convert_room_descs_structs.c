#include "convert_utils.h"

uint8_t convert_room_descs(void) {
  FILE *fp_in;
  FILE *fp_out;

  uint32_t bitfield;
  size_t size = 0;
  struct mheader_new mh;
  uint8_t *p;
    
  char *text;

  /* This comment is about updating the messages struct */
  /* It will rework some things and add some other things*/
  /* 11/25/2023 */

  /* Read all the room descriptions and convert those first */
  char filename[255];

  for (uint8_t i = 0; i < MAXROOMS; i++) {
    sprintf(filename, "%sroom%i", "/home/bbs/message.orig/desc/", i);
    fp_in = fopen(filename, "r");
    if (!fp_in) {
      printf("Room: %i is missing room description.\n", i);
      continue;
    }

    p = mmap_file(filename, &size);
    // zero it out
    memset(&mh, 0, sizeof(struct mheader_new));

    fread(&bitfield, 1, S32, fp_in);
    mh.magic = getbits(bitfield, 0, 8);

    if (mh.magic != M_MAGIC) {
      printf("Room: %i has invalid room description.\n", i);
      fclose(fp_in);
      continue;
    }

    mh.poster = getbits(bitfield, 8, 24);
    fread(&bitfield, 1, S32, fp_in);
    mh.quotedx = getbits(bitfield, 5, 1);
    mh.mail = getbits(bitfield, 6, 1);
    mh.approval = getbits(bitfield, 7, 1);
    mh.hlen = getbits(bitfield, 8, 6);

    mh.len = getbits(bitfield, 14, 18);
    fread(&mh.msgid, 1, S32, fp_in);
    fread(&mh.forum, 1, S16, fp_in);
    fread(&mh.mtype, 1, S8, fp_in);
    // move forward one
    fseek(fp_in, 1, SEEK_CUR);
    fread(&mh.ptime, 1, S32, fp_in);

    text = calloc(1, mh.len);
    fread(&bitfield, 1, S32, fp_in);
    mh.ext.mail.recipient = getbits(bitfield, 8, 24);
    // seek to where the data starts */
    fseek(fp_in, mh.hlen, SEEK_SET);
    fread(text, mh.len, S8, fp_in);
    fclose(fp_in);
    sprintf(filename, "convert_output/message/desc/room%i", i);
    fp_out = fopen(filename, "w");
    mh.hlen = sizeof(struct mheader_new);
    fwrite(&mh, mh.hlen, 1, fp_out);

    fwrite(text, mh.len, 1, fp_out);
    free(text);
    fclose(fp_out);
    munmap(p, size);
  }
  return 0;
}