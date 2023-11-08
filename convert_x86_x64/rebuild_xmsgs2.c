#include "convert_utils.h"

#define FILE_START 4
#define CONV_MAX 65535

uint16_t converted_msgs;
uint32_t pos_map[CONV_MAX][2];

uint32_t lookup_xmsg_loc(uint32_t xloc) {
  for (uint16_t i = 0; i < converted_msgs; i++) {
    if (pos_map[i][0] == xloc) {
      return pos_map[i][1];
    }
  }
  return 0;
}
int32_t get_next_message(bool sender, struct xheader xh) {
  int32_t xmsgs_loc;
  if (sender) {
    xmsgs_loc = xh.sprev;
  } else {
    xmsgs_loc = xh.rprev;
  }
  return xmsgs_loc;
}

bool get_sender(struct xheader xh, struct user *source) {
  bool sender = true;
  // get the next message location
  if (xh.snum != source->usernum) {
    sender = false;
    if (xh.rnum != source->usernum) {
      printf(
          "Message points to a location that is not the sender or "
          "receiver.\n");
      sender = false;
    }
  } else if (xh.rnum == source->usernum) {
    printf("You need to handle this.\n");
  }
  return sender;
}

void store_xmsg_map(uint32_t oldloc, uint32_t newloc) {
  pos_map[converted_msgs][0] = oldloc;
  pos_map[converted_msgs][1] = newloc;
  converted_msgs++;
  if (converted_msgs > CONV_MAX) {
    printf("Exceeded message conversion allocation. Expect seg fault.\n");
    sleep(2);
  }
}

uint16_t get_xmsgs(FILE *fp, struct xheader *xh, char *text, int loc) {
  uint16_t idx = 0;
  if (fseek(fp, loc, SEEK_SET)) {
    xh->checkbit = 0;
    return 0;
  }
  fread(xh, 1, sizeof(struct xheader), fp);
  if (!xh->checkbit) {
    return 0;
  }
  while (1) {
    fread(&text[idx++], 1, S8, fp);
    if (idx > 1 && !text[idx - 1] && !text[idx - 2]) {
      return idx;
    }
  }
}

uint8_t rebuild_xmsgs(void) {
  uint32_t file_position = 4;
  struct userdata *udata;
  struct user *source;
  FILE *fp_orig;
  size_t size = 0;
  uint8_t *mx_new, *mx_prev, *mx_origin;
  struct msg *msg;
  uint32_t which;
  uint32_t totalusers;
  uint32_t link;
  int fd;
  struct xheader xh, *xh_prev, xh_check;
  char text[4192];
  uint16_t tlen;
  int32_t xmsgs_loc, xmsgs_previous_loc, new_loc, upd_loc;
  int32_t new_offset;
  uint32_t loc_start = FILE_START;
  bool sender = true, previous_sender;

  /* for remap lookups */

  // Open the user files.. because we're going to be making a lot of changes
  // and this will be easier using the mmap values
  udata = get_users();

  if (!udata) {
    printf("Cannot get udata.\n");
    return 0;
  }

  which = udata->which;
  totalusers = udata->totalusers[which];

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

  mx_origin = mx_new;
  // The file origin is where data will start to be written.
  // which is the pointer to in the message structure
  mx_new += loc_start;

  /* Find the valid users. */
  for (uint16_t i = 0; i < totalusers; i++) {
    link = udata->num[which][i];
    source = (struct user *)(udata + 1) + (link);

/*
    if (!(source->usernum == 1139 || source->usernum == 5)) {
      continue;
    }*/
    if (!source->xmaxpos) {
      continue;
    }

    if (source->usernum == 1139) {
      printf("Dredd\n");
    }

    // get the newest message
    xmsgs_loc = source->xmaxpos;

    while (xmsgs_loc) {
      if ((new_loc = lookup_xmsg_loc(xmsgs_loc))) {
        printf("This location (%#010x), has already been copied to %#010x\n",
               xmsgs_loc, new_loc);

        if (xmsgs_loc == source->xmaxpos) {
          source->xmaxpos = new_loc;
          xmsgs_previous_loc = new_loc;
        } else {
          memcpy(&xh_check, (void *)mx_origin + xmsgs_previous_loc,
                 sizeof(struct xheader));
          /* update any and all references */
          upd_loc = lookup_xmsg_loc(xh_check.sprev);
          if (upd_loc) {
            xh_check.sprev = upd_loc;
          }
          upd_loc = lookup_xmsg_loc(xh_check.snext);
          if (upd_loc) {
            xh_check.snext = upd_loc;
          }
          upd_loc = lookup_xmsg_loc(xh_check.rprev);
          if (upd_loc) {
            xh_check.rprev = upd_loc;
          }
          upd_loc = lookup_xmsg_loc(xh_check.rnext);
          if (upd_loc) {
            xh_check.rnext = upd_loc;
          }

          memcpy(mx_origin + xmsgs_previous_loc, &xh_check,
                 sizeof(struct xheader));
          xmsgs_previous_loc = new_loc;
        }
        // Get this (original) message location into memory
        fseek(fp_orig, xmsgs_loc, SEEK_SET);
        fread(&xh_check, 1, sizeof(struct xheader), fp_orig);

        sender = get_sender(xh_check, source);
        previous_sender = sender;
        xmsgs_loc = get_next_message(sender, xh_check);
        // this is the location of the previous message in the chain
        mx_prev = mx_origin + new_loc;
        continue;
      }

      memset(&xh, 0, sizeof(struct xheader));

      tlen = get_xmsgs(fp_orig, &xh, text, xmsgs_loc);

      // If the location is bad, then set the pointer to 0.
      // if it's down the message chain somewhere that's the end.

      printf("Current: %#010x, prev: %#010x, next: %#010x, \n", xmsgs_loc,
             xh.sprev, xh.snext);
      printf("    Users:  Sender: %i, Recv: %i\n", xh.snum, xh.rnum);
      if (!tlen || (source->usernum != xh.snum && source->usernum != xh.rnum)) {
        printf("\nThis is a bad message location: %i %#010x\n", xmsgs_loc,
               xmsgs_loc);

        if (xmsgs_loc == source->xmaxpos) {
          // Set to zero for no history or bad history
          source->xmaxpos = 0;
        } else {
          source->xminpos = xmsgs_previous_loc;
        }
        break;
      }
      printf("   ");
      for (uint16_t i = 0; i < ((tlen > 60) ? 60 : tlen); i++) {
        printf("%c", text[i] ? text[i] : ' ');
      }
      printf("\n");

      sender = get_sender(xh, source);

      // If we've moved forward at all then we need to update the prior record
      if (file_position > loc_start) {
        xh_prev = (struct xheader *)mx_prev;
        if (previous_sender) {
          xh_prev->sprev = file_position;
        } else {
          xh_prev->rprev = file_position;
        }
      } else {
        // the first user gets the first entry
        source->xmaxpos = loc_start;
      }
      // store the current location and the new location in memory
      store_xmsg_map(xmsgs_loc, file_position);
      // now we can update current location to the new
      xmsgs_loc = file_position;

      // copy the data to the new file
      memcpy(mx_new, &xh, sizeof(struct xheader));
      memcpy(mx_new + sizeof(struct xheader), text, tlen);

      mx_prev = mx_new;

      // move to a new place in the file, and file position
      mx_new += sizeof(struct xheader) + tlen;
      // move the file position forward
      file_position += sizeof(struct xheader) + tlen;

      xmsgs_previous_loc = xmsgs_loc;

      xmsgs_loc = get_next_message(sender, xh);
      previous_sender = sender;
    }
  }

  // Pointer to the next post.
  msg->xcurpos = file_position;
  printf("Final location: %#010lx\n", msg->xcurpos);
  munmap(mx_new, XMSGSIZE);
  munmap(msg, sizeof(struct msg));
  fclose(fp_orig);

  return 0;
}
