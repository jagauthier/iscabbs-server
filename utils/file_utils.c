#include "defs.h"
#include "ext.h"

struct userdata *get_users(char *filename) {
  size_t size;
  int f;
  struct userdata *udata;

  if ((f = open(filename, O_RDWR)) < 0) {
    printf("Cannot open %s\n", filename);
    return NULL;
  }

  size = lseek(f, 0, SEEK_END);

  udata = (struct userdata *)mmap(0, size, PROT_READ | PROT_WRITE,
                                  MAP_SHARED | MAP_FILE, f, 0);
  if (!udata) {
    printf("udata bad.\n");
    return NULL;
  }
  close(f);
  return udata;
}

unsigned char *get_msgmain(char *filename) {
  unsigned char *msgstart;
  size_t size;
  size = 0;
  msgstart = mmap_file(filename, &size);
  /* Debugger shows me this value is correct!!*/
  return msgstart;
}

struct user *find_user(struct userdata *udata, const char *name,
                       uint32_t usernum) {
  struct user *source;
  uint16_t link;
  uint32_t totalusers;
  uint16_t which = udata->which;

  totalusers = udata->totalusers[which];

  if (!name && !usernum) {
    return NULL;
  }
  for (uint16_t i = 0; i < totalusers; i++) {
    link = udata->num[which][i];
    source = (struct user *)(udata + 1) + (link);
    if (usernum) {
      if (source->usernum == usernum) {
        return source;
      }
    } else {
      if (!strcmp(source->name, name) && name) {
        return source;
      }
    }
  }
  return NULL;
}


