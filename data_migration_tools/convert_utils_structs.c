#include "convert_utils.h"

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

uint8_t *get_users() {
  int f;
  off_t size;
  uint8_t *udata;
  if ((f = open("convert_output/data/userdata", O_RDWR)) < 0) {
    perror("openuser open");
    return NULL;
  }
  
  size = lseek(f, 0, SEEK_END);
  printf("size: %i\n", size);

  udata = mmap(
      0, size,
      PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, f, 0);
  if (!udata) {
    return NULL;
  }
  close(f);
  return udata;
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
