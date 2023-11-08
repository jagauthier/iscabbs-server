#include "convert_utils.h"

#define FILE_START 4
#define CONV_MAX 65534
#define MAX_X_SIZE 2048

#define OLDXMSGSIZE 5000000

#define DEBUG_USER 0

uint16_t converted_msgs;
uint32_t pos_map[CONV_MAX][2];

struct userdata *udata;

bool valid_user(uint32_t usernum) {
  struct user *source;
  uint32_t which;
  uint32_t totalusers;
  uint32_t link;
  which = udata->which;
  totalusers = udata->totalusers[which];
  for (uint16_t i = 0; i < totalusers; i++) {
    link = udata->num[which][i];
    source = (struct user *)(udata + 1) + (link);
    if (source->usernum == usernum) {
      return true;
    }
  }
  return false;
}

uint32_t lookup_xmsg_loc(char *debug, uint32_t xloc) {
  if (xloc) {
    for (uint16_t i = 0; i < converted_msgs; i++) {
      if (pos_map[i][0] == xloc) {
        return pos_map[i][1];
      }
    }
    if (DEBUG_USER) {
      printf("Requested invalid mapping (%s): %i\n", debug, xloc);
    }
  }
  return 0;
}

void update_users_pos(void) {
  struct user *source;
  uint32_t which;
  uint32_t totalusers;
  uint32_t link;
  which = udata->which;
  totalusers = udata->totalusers[which];
  for (uint16_t i = 0; i < totalusers; i++) {
    link = udata->num[which][i];
    source = (struct user *)(udata + 1) + (link);
    if (DEBUG_USER && source->usernum != DEBUG_USER) {
      continue;
    }
    source->xmaxpos = lookup_xmsg_loc("xmaxpos", source->xmaxpos);
    source->xminpos = lookup_xmsg_loc("xminpos", source->xminpos);

    if (!source->xmaxpos && DEBUG_USER) {
      // printf("User '%s' has bad starting X pointer.\n", source->name);
    }
    if (!source->xminpos && DEBUG_USER) {
      // printf("User '%s' has bad ending X pointer.\n", source->name);
    }
  }
}

void store_xmsg_map(uint32_t oldloc, uint32_t newloc) {
  pos_map[converted_msgs][0] = oldloc;
  pos_map[converted_msgs][1] = newloc;
  converted_msgs++;
  if (converted_msgs > CONV_MAX) {
    printf("Exceeded message conversion allocation. Expect seg fault.\n");
    sleep(2);
  }
  // printf("Storing old to new: %i %i\n", oldloc, newloc);
}

/* this function reads the xmessages from the xmsgs database*/

/* It also is used after the conversion */
/* since there is a time_t we need a before/after check */

uint16_t get_xmsgs(FILE *fp, struct xheader *xh, char *text, bool after) {
  uint16_t idx = 0;
  uint32_t start = ftell(fp);
  uint32_t bitfield;
  uint8_t read_size;

  /* Can just read the struct after wards because this is the converted file*/
  if (after) {
    fread(xh, 1, sizeof(struct xheader), fp);
    // printf("loc: %i\n", ftell(fp));
    // fseek(fp, -sizeof(struct xheader), SEEK_CUR);

  } else {
    fread(&bitfield, 1, S32, fp);
    xh->checkbit = getbits(bitfield, 0, 1);
    xh->type = getbits(bitfield, 7, 3);

    fread(&xh->snum, 1, S32, fp);
    fread(&xh->rnum, 1, S32, fp);
    fread(&xh->sprev, 1, S32, fp);
    fread(&xh->snext, 1, S32, fp);
    fread(&xh->rprev, 1, S32, fp);
    fread(&xh->rnext, 1, S32, fp);
    fread(&xh->time, 1, S32, fp);
  }
  if (!xh->checkbit) {
    return 0;
  }

  if (xh->type < X_NORMAL || xh->type > X_BROADCAST) {
    return 0;
  }
  // If they are both invalid users, naw.
  if (!valid_user(xh->snum) && !valid_user(xh->rnum)) {
    return 0;
  }

  /* But still - somehow this gets through all the checks */
  /* So finally, we'll check that each message pointer is valid */
  if (xh->sprev > OLDXMSGSIZE || xh->snext > OLDXMSGSIZE ||
      xh->rnext > OLDXMSGSIZE || xh->rprev > OLDXMSGSIZE) {
    return 0;
  }

  /* AND EVEN SO - it still gets here.  If this check fails... I dunno */
  /* check that the time not in the future, nor before 01/01/2020 */
  if (xh->time > time(0) || xh->time < 1420070400) {
    return 0;
  }

  // printf("loc: %i\n", ftell(fp));
  while (1) {
    fread(&text[idx++], 1, S8, fp);
    if (idx > MAX_X_SIZE) {
      printf("Exceeded X size. (pos: %i)\n", start);
      printf("  %i %i\n", xh->snum, xh->rnum);
      printf("  %s\n", text);
      return 0;
    }
    if (idx > 1 && !text[idx - 1] && !text[idx - 2]) {
      return idx;
    }
  }
}

uint8_t rebuild_xmsgs(void) {
  uint32_t file_position = 4;

  FILE *fp_orig, *fp_new;
  size_t size = 0;
  uint8_t *mx_new;
  struct msg *msg;

  int fd;
  struct xheader xh;
  char text[MAX_X_SIZE];
  uint16_t tlen;
  uint32_t loc, msg_start;

  // Open the user files.. because we're going to be making a lot of changes
  // and this will be easier using the mmap values
  udata = get_users();

  if (!udata) {
    printf("Cannot get udata.\n");
    return 0;
  }

  /* open the original file for copying */
  fp_orig = fopen("/home/bbs/data.orig/xmsgdata", "r");
  if (!fp_orig) {
    printf("Could not open %s\n", "/home/bbs/data.orig/xmsgdata");
    return 1;
  }

  /* Create a new empty file*/
  size = XMSGSIZE;

  fd = open("convert_output/data/xmsgdata", O_WRONLY | O_CREAT, 0660);
  ftruncate(fd, size);
  close(fd);
  mx_new = mmap_file("convert_output/data/xmsgdata", &size);

  /* open the message file so we can update the xcurpos later */
  size = sizeof(struct msg);
  if (!(msg = (struct msg *)mmap_file("convert_output/data/msgdata", &size))) {
    my_printf("msgdata\n");
    return 0;
  }
  memset(mx_new, 0, XMSGSIZE);

  // The file origin is where data will start to be written.
  // which is the pointer to in the message structure
  mx_new += FILE_START;

  fseek(fp_orig, FILE_START, SEEK_SET);

  loc = ftell(fp_orig);

  while (loc < OLDXMSGSIZE) {
    loc = ftell(fp_orig);
    if (loc >= 15415) {
    }
    memset(&xh, 0, sizeof(struct xheader));
    tlen = get_xmsgs(fp_orig, &xh, text, false);

    if (tlen) {
      if (DEBUG_USER && (!(xh.snum == DEBUG_USER || xh.rnum == DEBUG_USER))) {
        continue;
      }

      msg_start = loc;
      loc = ftell(fp_orig);
      // printf("Found a message. (Num: %i, Pos: %i, Len: %i)\n",
      // converted_msgs,
      //        msg_start, tlen);

      // printf("   %i %i\n", xh.snum, xh.rnum);

    } else {
      fseek(fp_orig, loc + 1, SEEK_SET);
      continue;
    }
    // printf("Creating mapping: %i %i\n", msg_start, file_position);
    store_xmsg_map(msg_start, file_position);

    // copy the data to the new file
    memcpy(mx_new, &xh, sizeof(struct xheader));
    memcpy(mx_new + sizeof(struct xheader), text, tlen);

    // move to a new place in the file, and file position
    mx_new += sizeof(struct xheader) + tlen;
    // move the file position forward
    file_position += sizeof(struct xheader) + tlen;
  }

  // Pointer to the next post.
  msg->xcurpos = file_position;
  printf("Final location: %#010lx\n", msg->xcurpos);
  munmap(msg, sizeof(struct msg));
  munmap(mx_new, XMSGSIZE);
  fclose(fp_orig);
  /* Now go back and update the users as well as the file pointer locations */
  update_users_pos();

  printf("Converting new file positions.\n");

  /* open the original file for copying */
  fp_new = fopen("convert_output/data/xmsgdata", "r+");
  if (!fp_new) {
    printf("Could not open %s\n", "convert_output/data/xmsgdata");
    return 1;
  }

  fseek(fp_new, FILE_START, SEEK_SET);

  loc = ftell(fp_new);

  while (loc < file_position) {
    loc = ftell(fp_new);
    if (loc >= 5221095) {
      
    }
    msg_start = loc;
    memset(&xh, 0, sizeof(struct xheader));
    tlen = get_xmsgs(fp_new, &xh, text, true);

    if (tlen) {
      loc = ftell(fp_new);
      // printf("Found new message: (Pos: %i, Len: %i)\n",
      //        msg_start, tlen);
      // printf("   %i %i\n", xh.snum, xh.rnum);
      xh.rprev = lookup_xmsg_loc("rprev", xh.rprev);
      xh.sprev = lookup_xmsg_loc("sprev", xh.sprev);
      xh.rnext = lookup_xmsg_loc("rnext", xh.rnext);
      xh.snext = lookup_xmsg_loc("snext", xh.snext);
      fseek(fp_new, msg_start, SEEK_SET);
      fwrite(&xh, 1, sizeof(struct xheader), fp_new);
      fseek(fp_new, loc, SEEK_SET);
    } else {
      fseek(fp_new, loc + 1, SEEK_SET);
      continue;
    }
  }
  printf("Last known file location: %i\n", ftell(fp_new));
  fclose(fp_new);
  return 0;
}
