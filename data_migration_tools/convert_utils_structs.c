#include "convert_utils.h"

uint32_t *magic_locations;
uint32_t magics;

unsigned getbits(uint32_t value, uint8_t offset, uint8_t n) {
  const unsigned max_n = CHAR_BIT * sizeof(unsigned);
  if (offset >= max_n)
    return 0;       /* value is padded with infinite zeros on the left */
  value >>= offset; /* drop offset bits */
  if (n >= max_n) return value;        /* all  bits requested */
  const unsigned mask = (1u << n) - 1; /* n '1's */
  return value & mask;
}

/* load the converted users into the user struct */

struct userdata *get_users(void) {
  int f;
  struct userdata *udata;
  if ((f = open("convert_output/data/userdata", O_RDWR)) < 0) {
    perror("openuser open");
    return NULL;
  }
  udata = (struct userdata *)mmap(
      0, sizeof(struct userdata) + sizeof(struct user) * MAXTOTALUSERS,
      PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, f, 0);
  if (!udata || udata == (struct userdata *)-1) {
    return NULL;
  }
  close(f);
  return (udata);
}

/* find users but using a non-global udata */
struct user *my_finduser(const char *name, uint32_t usernum, int linknum,
                         struct userdata *udata) {
  int32_t lower = 0;
  int32_t upper = 0;
  int32_t mid = 0;
  int32_t cmp = 0;

  int saveindex = 0;
  int savelinknum;

  if (!name && !usernum) {
    return ((struct user *)(udata + 1) + linknum);
  }

  for (int gen = -1; gen != udata->gen;) {
    if (gen >= 0) {
      udata->retries++;
    }
    gen = udata->gen;
    int32_t old = udata->which;
    if (gen != udata->gen) {
      continue;
    }

    mid = cmp = 0;
    for (lower = 0, upper = udata->totalusers[old] - 1; lower <= upper;) {
      mid = (lower + upper) >> 1;
      linknum = name ? udata->name[old][mid] : udata->num[old][mid];
      struct userlink *linkptr = &udata->link[linknum];
      cmp = name ? (uint32_t)strcmp(name, linkptr->name)
                 : (usernum - linkptr->usernum);

      if (cmp < 0) {
        upper = mid - 1;
      } else if (cmp > 0) {
        lower = mid + 1;
      } else {
        break;
      }
    }
  }

  saveindex = mid + (cmp > 0);
  if (lower > upper) {
    return (NULL);
  } else {
    return ((struct user *)(udata + 1) + (savelinknum = linknum));
  }
}

/* open the original file and count messages and store the locations */
/* so then we can see how many to calculate the reposition */

uint8_t find_magics(void) {
  uint8_t *mfp;
  FILE *fp;
  size_t size;
  uint32_t mfp_pos, count;
  struct mheader *p;

  fp = fopen("/home/bbs/message.orig/msgmain", "r");
  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  mfp = calloc(1, size);
  if (!mfp) {
    printf("Could not allocate memory for msgmain buffer.\n");
    return 1;
  }
  fread(mfp, size, S8, fp);
  fclose(fp);

  for (mfp_pos = 0; mfp_pos < size; mfp_pos++) {
    p = (struct mheader *)(void *)(mfp + mfp_pos);
    if (p->magic == M_MAGIC) {
      mfp_pos += p->hlen + p->len;
      count++;
    }
  }
  magics = count;
  /* allocate the memory to store the locations */
  magic_locations = calloc(sizeof(uint32_t), count);
  count = 0;
  for (mfp_pos = 0; mfp_pos < size; mfp_pos++) {
    p = (struct mheader *)(void *)(mfp + mfp_pos);
    if (p->magic == M_MAGIC) {
      *(magic_locations + count) = (mfp_pos);
      mfp_pos += p->hlen + p->len;
      count++;
    }
  }
  free(mfp);
  return 0;
}

int32_t find_magic_location(uint32_t pos) {
  uint32_t low = 0, mid = 0, high = magics, value;

  while (low <= high) {
    mid = low + (high - low) / 2;
    value = *(magic_locations + mid);

    // printf("%i: %#x (%i), %#x (%i)\n", mid, value, value, pos, pos);

    if (value == pos) {
      return mid;
    }

    if (value < pos) {
      low = mid + 1;
    } else {
      high = mid - 1;
    }
  }
  return mid;
}

uint32_t count_magics(uint32_t pos, uint8_t *mfp) {
  struct mheader *p;
  uint32_t mfp_pos, count = 0;
  for (mfp_pos = 0; mfp_pos < pos; mfp_pos++) {
    // if (*(mfp + mfp_pos) == M_MAGIC) {
    p = (struct mheader *)(void *)(mfp + mfp_pos);
    if (p->magic == M_MAGIC) {
      mfp_pos += p->hlen + p->len;
      count++;
    }
  }
  return count;
}