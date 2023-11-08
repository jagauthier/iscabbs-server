#include "convert_utils.h"
unsigned getbits(uint32_t, uint8_t, uint8_t);

uint8_t rebuild_messages(void) {
  int32_t fd;
  uint32_t bitfield;
  size_t size = 0;
  struct mheader mh;
  uint8_t *mm_new;
  uint32_t mm_origin, file_position, new_file_position;
  char filename[255];

  char *text;
  uint8_t magic;
  FILE *fp_orig;

  struct msg *msg;

  fp_orig = fopen("/home/bbs/message.orig/msgmain", "r");
  if (!fp_orig) {
    printf("Could not open %s\n", "/home/bbs/message.orig/msgmain");
    return 1;
  }
  size = MM_FILELEN;

  sprintf(filename, "convert_output/message/msgmain");
  fd = open(filename, O_WRONLY | O_CREAT, 0660);
  ftruncate(fd, size);
  close(fd);
  mm_new = mmap_file(filename, &size);

  size = sizeof(struct msg);
  if (!(msg = (struct msg *)mmap_file("convert_output/data/msgdata", &size))) {
    my_printf("msgdata\n");
    return 0;
  }
  memset(mm_new, 0, MM_FILELEN);

  mm_origin = (uintptr_t)mm_new;
  /* loop through the rooms. Validate they point to legitamate messages
  in the original file. If not, clear that.  If so. copy that data
  to the new file and change the pointer. Simple right?

  Then, figure out how mail works?

  Lastly, change the pointer for the next post location (curpos) */
  for (uint8_t room = 0; room < MAXROOMS; room++) {
    // no mail

    for (uint8_t post = 0; post < MSGSPERRM; post++) {
      if (msg->room[room].num[post] && msg->room[room].pos[post]) {

        fseek(fp_orig, msg->room[room].pos[post], SEEK_SET);
        // printf("Copying message at location %#010x\n",
        //        (uint32_t)ftell(fp_orig));
        fread(&magic, 1, S8, fp_orig);
        if (magic != M_MAGIC) {
          // printf("Room: %i, Message index %i, location %#010x: is not
          // MAGIC.\n", room, post, msg->room[room].pos[post]);
          msg->room[room].num[post] = msg->room[room].pos[post] = 0;
          continue;
        }
        // move it back
        fseek(fp_orig, -1, SEEK_CUR);

        /* Success, so copy the data */
        memset(&mh, 0, sizeof(struct mheader));
        fread(&bitfield, 1, S32, fp_orig);
        mh.magic = getbits(bitfield, 0, 8);
        if (mh.magic != M_MAGIC) {
          printf("It's not magic now. How?\n");
          return 0;
        }

        mh.poster = getbits(bitfield, 8, 24);
        fread(&bitfield, 1, S32, fp_orig);
        mh.quotedx = getbits(bitfield, 5, 1);
        mh.mail = getbits(bitfield, 6, 1);
        mh.approval = getbits(bitfield, 7, 1);
        mh.hlen = getbits(bitfield, 8, 6);
        if (mh.hlen > 20) {
          // printf("hlen: %i: %ld bytes in\n", mh.hlen, ftell(fp_in));
        }
        mh.len = getbits(bitfield, 14, 18);
        fread(&mh.msgid, 1, S32, fp_orig);
        fread(&mh.forum, 1, S16, fp_orig);
        fread(&mh.mtype, 1, S8, fp_orig);
        // move forward one
        fseek(fp_orig, 1, SEEK_CUR);

        fread(&mh.ptime, 1, S32, fp_orig);
        text = calloc(1, mh.len + 1);
        if (!text) {
          printf("Couldn't allocate any memory\n");
          return 1;
        }
        fread(&bitfield, 1, S32, fp_orig);
        mh.ext.mail.recipient = getbits(bitfield, 8, 24);
        fseek(fp_orig, msg->room[room].pos[post] + mh.hlen, SEEK_SET);
        // printf("Copying text at location: %#010x\n",
        // (uint32_t)ftell(fp_orig));
        fread(text, mh.len, S8, fp_orig);
        text[mh.len] = 0;

        if (!x86) {
          // because ptime is now 8 bytes and not 4.
          mh.hlen += S32;
        }

        memcpy(mm_new, &mh, mh.hlen);
        memcpy(mm_new + mh.hlen, text, mh.len);

        // Calculate the file position based on the mmap and start.
        // below the mmap location will increment to the next location
        file_position = (uintptr_t)mm_new - mm_origin;
        // update the message to point to new location
        msg->room[room].pos[post] = file_position;

        // Move the pointer forward, and add 1 to get a 0 after the string */
        mm_new += mh.hlen + strlen(text) + 1;

        // This is where it should go
        file_position = (uintptr_t)mm_new - mm_origin;

        if (file_position % 4 != 0) {
          // printf("%#010x is not 32b aligned\n", new_file_position);
        }

        new_file_position = (file_position + 4 - file_position % 4);
        if (new_file_position % 4 == 0) {
          // printf("%#010x is now 32b aligned\n", new_file_position);
        }
        // move forward the difference?
        mm_new += new_file_position - file_position;
        // printf("File position next: %#010x\n", file_position);
      }
    }
  }

  
  // Pointer to the next post.
  msg->curpos = (uintptr_t)mm_new - mm_origin;
  printf("Final location: %#010lx\n", msg->curpos);
  munmap(mm_new, MM_FILELEN);
  munmap(msg, sizeof(struct msg));
  fclose(fp_orig);
  return 0;
}
