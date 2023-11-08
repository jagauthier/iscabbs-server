#include "convert_utils.h"

void main(void) {
  size_t size;
  uint32_t location = 4;
  uint8_t *mx_new;
  size = XMSGSIZE;
  uint8_t *c;
  struct xheader *xhm, *xh;
  struct xheader xhc;


  int fd = open("convert_output/data/xmsgdata", O_RDWR);
  if (fd == -1) {
    printf("could not open");
    return;
  }

  size = lseek(fd, 0, SEEK_END);
  // mmap, close, then validate.
  mx_new =
      mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, fd, 0);
  close(fd);

  memcpy(&xhc, (void *)mx_new + location, sizeof(struct xheader));
  xhc.snum = 0x99;
  memcpy(mx_new + location, &xhc, sizeof(struct xheader));
  printf("check: %i\n", xh->checkbit);
}