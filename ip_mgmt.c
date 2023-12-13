#include "defs.h"
#include "ext.h"

#include <regex.h>

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

int check_ip_addr(const char* ip) {
    regex_t regex;
    int ret;

    // Regular expression pattern to match IP address with wildcards
    const char* pattern = "^\\(\\([0-9]\\{1,3\\}\\|\\*\\)\\.\\)\\{3\\}\\([0-9]\\{1,3\\}\\|\\*\\)$";

    // Compile the regular expression
    ret = regcomp(&regex, pattern, REG_EXTENDED);
    if (ret != 0) {
        printf("Failed to compile regex\n");
        return -1;
    }

    // Execute the regular expression
    ret = regexec(&regex, ip, 0, NULL, 0);
    if (ret == 0) {
        printf("Valid IP address\n");
    } else if (ret == REG_NOMATCH) {
        printf("Invalid IP address\n");
    } else {
        regerror(ret, &regex, NULL, 0);
        printf("Regex match failed\n");
    }

    // Free regex resources
    regfree(&regex);

    return ret;
}
void add_host() {
    char ip[15];
    colorize("@Y:Enter the IPv4 address to block. You can use @G*@Y as wildcard. ->");
    get_string("", 15, ip, -1);
}

void ip_mgmt_menu(void) {
  char chr = '?';

  while (chr != ' ' && chr != '\n') {
    switch (chr) {
      case 'A':
        add_host();
        break;

      case 'D':
       // delete_host();
        break;

      case 'V':
       // view_hosts();
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
