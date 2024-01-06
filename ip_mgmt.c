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

  blocklist = (struct blocklist*)mmap(0, sizeof(struct blocklist),
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
  bool valid = false;

  // Regular expression pattern to match IP address with wildcards
  const char* pattern =
      "^([0-9]{1,2}|1[0-9]{2}|2[0-4][0-9]|25[0-5]|\\*)(\\.([0-9]{1,2}|1[0-9]{2}"
      "|2[0-4][0-9]|25[0-5]|\\*)){3}$";

  // Compile the regular expression
  ret = regcomp(&regex, pattern, REG_EXTENDED);
  if (ret) {
    printf("Failed to compile regex\n");
    return false;
  }

  // Execute the regular expression
  ret = regexec(&regex, ip, 0, NULL, 0);
  if (ret == 0) {
    valid = true;
  } else if (ret == REG_NOMATCH) {
    colorize(BOLD_RED"Invalid IP address.\n");
    valid = false;
  } else {
    regerror(ret, &regex, NULL, 0);
    colorize(BOLD_RED"Regex match failed\n");
    valid = false;
  }

  // Free regex resources
  regfree(&regex);

  return valid;
}

void add_ip_blocklist(char* ip, char* reason) {
  uint16_t i;

  /* loop through the block list looking for the first empty entry*/
  for (i = 0; i < MAX_IPS; i++)
    if (blocklist->usernum[i] == 0) {
      blocklist->usernum[i] = ouruser->usernum;
      strcpy(blocklist->name[i], ouruser->name);
      strcpy(blocklist->reason[i], reason);
      strcpy(blocklist->ip[i], ip);
      blocklist->blocked_time[i] = time(0);
      break;
    }

  colorize(BOLD_GREEN"IP block added at index %i\n", i);
}

void add_blocklist() {
  char ip[15];
  char reason[60];
  bool valid_ip = false;
  while (!valid_ip) {
    ip[0] = 0;
    colorize(
        BOLD_YELLOW"\nEnter the IPv4 address to block. You can use "BOLD_GREEN"*"BOLD_YELLOW" as wildcard. "
        "-> "BOLD_MAGENTA);
    get_string("", 15, ip, -1);
    if (ip[0] == 0) {
      return;
    };
    valid_ip = check_ip_addr(ip);
    if (!strcmp(ip, "*.*.*.*")) {
      colorize(BOLD_RED"That's a really bad idea.\n");
      return;
    }
  }
  colorize(BOLD_YELLOW"Please provide a description to justify the block ->\n"BOLD_MAGENTA);
  reason[0] = 0;
  get_string("", 60, reason, -1);
  if (reason[0] == 0) {
    colorize(BOLD_RED"Entry aborted.\n");
    return;
  }
  add_ip_blocklist(ip, reason);
}
void view_blocklist() {
  char* b_string;
  uint16_t items = 0, i;

  for (i = 0; i < MAX_IPS; i++) {
    if (blocklist->usernum[i] > 0) {
      items++;
    }
  }

  if (!items) {
    colorize(BOLD_YELLOW"There are currently no IPs blocked.\n"BOLD_GREEN);
    return;
  }

  // for pagination
  b_string = calloc(items + 2, 160);
  b_string = my_sprintf(NULL, BOLD_WHITE"\n%3s "BOLD_CYAN"%-15s"BOLD_YELLOW"%-20s"BOLD_RED"%4s "BOLD_BLUE"%-14s\n", "#",
                        "IP", "Name", "Hits", "Date/Time");
  b_string = my_sprintf(b_string, 
                        BOLD_GREEN"----------------------------------------------------"
                        "-------------\n");

  for (i = 0; i < MAX_IPS; i++) {
    if (blocklist->usernum[i] > 0) {
      b_string =
          my_sprintf(b_string, BOLD_WHITE"%3i "BOLD_CYAN"%-15s"BOLD_YELLOW"%-20s"BOLD_RED"%4i "BOLD_BLUE"%-14s\n", i,
                     blocklist->ip[i], blocklist->name[i], blocklist->hits[i],
                     formtime(3, blocklist->blocked_time[i]));
      b_string =
          my_sprintf(b_string, "    "WHITE"Reason: "BOLD_MAGENTA"%s\n\n", blocklist->reason[i]);
    }
  }
  display_with_pagination(b_string, rows, 1);

  free(b_string);
}

void delete_blocklist(void) {
  char nbr_str[4];
  uint16_t index;
  while (1) {
    view_blocklist();
    colorize(BOLD_GREEN);
    get_string("Delete entry by number [enter to quit] -> ", 3, nbr_str, -1);

    if (nbr_str[0] == 0) {
      break;
    }
    index = atoi(nbr_str);
    if (!blocklist->usernum[index]) {
      colorize(BOLD_RED"Not a valid entry.\n\n");
      continue;
    }
    blocklist->usernum[index] = 0;
    blocklist->hits[index] = 0;
    blocklist->blocked_time[index] = 0;
    memset(blocklist->name[index], 0, MAXALIAS + 1);
    memset(blocklist->reason[index], 0, 60);
    memset(blocklist->ip[index], 0, 15);
    colorize(BOLD_WHITE"Blocklist removed.\n\n");
  }
}

void ip_mgmt_menu(void) {
  char chr = '?';

  while (chr != ' ' && chr != '\n') {
    switch (chr) {
      case 'A':
        my_printf("Add to blocklist\n");
        add_blocklist();
        colorize(BOLD_GREEN);
        help("ip_mgmnt_cmd", NO);
        break;

      case 'D':
        my_printf("Delete from blocklist\n");
        delete_blocklist();
        colorize(BOLD_GREEN);
        help("ip_mgmnt_cmd", NO);
        break;

      case 'V':
        my_printf("View blocklist\n");
        view_blocklist();
        colorize(BOLD_GREEN);
        help("ip_mgmnt_cmd", NO);
        break;

      case '?':
      case '/':
        colorize(BOLD_GREEN);
        help("ip_mgmnt_cmd", NO);
        break;

      default:
        break;
    }

    if (chr) {
      colorize(BOLD_GREEN"IP management command: ");
    }

    chr = get_single_quiet("ADV \n/?");
  }
}

int extract_ip_octets(const char* ip, int16_t* octets) {
  // Ensure the IP string and octets array are not NULL
  if (ip == NULL || octets == NULL) {
    return -1;  // Invalid input
  }

  // Make a copy of the IP string since strtok modifies the string
  char
      ip_copy[16];  // Maximum size for IPv4 address string plus null terminator
  strncpy(ip_copy, ip, sizeof(ip_copy));
  ip_copy[sizeof(ip_copy) - 1] = '\0';  // Ensure null termination

  char* token;
  uint8_t i = 0;

  // Tokenize the string using dot as the delimiter
  token = strtok(ip_copy, ".");
  while (token != NULL && i < 4) {
    if (strcmp(token, "*") == 0) {
      octets[i] = -1;  // Use -1 to represent the wildcard
    } else {
      int octet = atoi(token);
      if (octet < 0 || octet > 255) {
        return -1;  // Octet out of range
      }
      octets[i] = octet;
    }
    i++;
    token = strtok(NULL, ".");
  }

  // Check if we have exactly 4 octets
  if (i != 4) {
    return -1;  // Did not match all four octets
  }

  return 0;  // Success
}
bool check_ip_blocklist(in_addr_t ip) {
  uint8_t octects[4];
  int16_t blocked_octects[4];

  bool match = false;

  // convert the ip into octect
  octects[0] = ip & 0xFF;
  octects[1] = ip >> 8 & 0xFF;
  octects[2] = ip >> 16 & 0xFF;
  octects[3] = ip >> 24 & 0xFF;
  //printf("IP: %i.%i.%i.%i\n", octects[0], octects[1], octects[2], octects[3]);

  for (uint16_t i = 0; i < MAX_IPS; i++) {
    if (blocklist->usernum[i]) {
      match = true;
      if (extract_ip_octets(blocklist->ip[i], blocked_octects) == 0) {
        for (uint8_t o = 0; o < 4; o++) {
          if (blocked_octects[i] == -1) {
            continue;
          }
          if (blocked_octects[i] != octects[i]) {
            match = false;
          }
        }
        if (match) {
          // increment the hit match
          blocklist->hits[i]++;
          return match;
        }
      }
    }
  }
  return match;
}
