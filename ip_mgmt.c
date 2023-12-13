#include "defs.h"
#include "ext.h"

bool open_ip_blocklist(void) {
  int f;
  size_t size;
  size = sizeof(struct blocklist);
  if ((f = open(BLOCKLIST, O_RDWR)) < 0) {
    /* Doesn't exist, so create it */
    f = open(BLOCKLIST, O_WRONLY | O_CREAT, 0660);
    if (f < 0) {
      my_printf("error generating blocklist file.");
      return false;
    }
    ftruncate(f, size);
    close(f);
  }

  blocklist = (struct blocklist *)mmap(0, sizeof(struct blocklist),
                                       PROT_READ | PROT_WRITE,
                                       MAP_SHARED | MAP_FILE, f, 0);

  if (!blocklist) {
    my_printf("Error mapping blocklist");
    return false;
  }
  close(f);
  return true;
}

void ip_mgmt_menu(void) {
  char chr = '?';

  while (chr != ' ' && chr != '\n') {
    switch (chr) {
      case 'a':
        break;

      case 'd':
        break;

      case 'v':
        break;

      case '?':
      case '/':
        help("ip_mgmnt_cmd", NO);
        break;

      default:
        break;
    }

    if (chr) {
      colorize("IP management command: ");
    }

    chr = get_single_quiet("ADV \n/?");
  }
}
