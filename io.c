/* Routines for character input and output to the BBS */
#define _GNU_SOURCE
#include "io.h"

#include <regex.h>
#include <stdarg.h>
#include <stdio.h>

#include "defs.h"
#include "ext.h"

#ifndef BBS
int ansi = 0;
#endif

static int my_cputs(const char* s);
static int my_puts(const char* s);

/* fix % signs and emails and whatever else */
/* This allows user entered text to be sent to colorize()*/
void cleanup_strings(char* string) {
  if (strstr(string, "%")) {
    replace_substring(string, "%", "%%");
  }
}

void replace_substring(char* str, const char* oldSubstr,
                       const char* newSubstr) {
  int oldLen = strlen(oldSubstr);
  int newLen = strlen(newSubstr);
  int strLen = strlen(str);

  char result[strlen(str) * 2];  // Adjust the size as per your requirement

  int i, count = 0;
  for (i = 0; i < strLen; i++) {
    if (strstr(&str[i], oldSubstr) == &str[i]) {
      strcpy(&result[count], newSubstr);
      count += newLen;
      i += oldLen - 1;
    } else {
      result[count] = str[i];
      count++;
    }
  }
  result[count] = '\0';

  strcpy(str, result);
}

/* A standard printf -- no color code recognition. */
/* works best if output is not fflushed */
int my_printf(const char* fmt, ...) {
  // delegate to my_vsprintf
  va_list ap;
  va_start(ap, fmt);
  char* buf = my_vsprintf(NULL, fmt, ap);
  va_end(ap);

  // output, then discard the buffer
  int r = my_puts(buf);
  free(buf);
  return r;
}

char* my_vsprintf(char* prefix, const char* fmt, va_list ap) {
  // let vasprintf do all the work for us.
  char* buf = NULL;
  int n = vasprintf(&buf, fmt, ap);

  if (n < 0 || buf == NULL) {
    return strdup("");
  }
  if (prefix == NULL) {
    return buf;
  }

  // concat the prefix and buf.
  char* result = NULL;

  n = asprintf(&result, "%s%s", prefix, buf);
  free(prefix);
  if (n < 0 || result == NULL) {
    return strdup("");
  }
  return result;
}

char* my_sprintf(char* prefix, const char* fmt, ...) {
  // delegate to my_vsprintf
  va_list ap;
  va_start(ap, fmt);
  char* buf = my_vsprintf(prefix, fmt, ap);
  va_end(ap);
  return buf;
}

int checked_snprintf_with_traceinfo(const char* file, int line, char* out,
                                    size_t len, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  errno = 0;
  const int n = vsnprintf(out, len, fmt, ap);
  const int saved_errno = errno;
  va_end(ap);

  if (n < 0) {
    // error: I/O or system error.
    char* emsg = my_sprintf(NULL,
                            "FATAL: vsnprintf system or I/O error. "
                            "Called at file '%s':%d "
                            " errno said: %s",
                            file, line, strerror(saved_errno));
    logfatal(emsg);
  } else if ((size_t)n >= len) {
    // error: buffer overflow
    char* emsg =
        my_sprintf(NULL, "FATAL: buffer overflow at file '%s':%d", file, line);
    logfatal(emsg);
  }
  return n;
}

char* checked_strcat_with_traceinfo(const char* file, int line, char* dest,
                                    size_t max_dest_size, const char* src) {
  // we need room for both strings and the null terminator.
  size_t src_len = strlen(src);
  size_t dest_len = strlen(dest);
  size_t need_size = dest_len + src_len + 1;

  if (need_size <= max_dest_size) {
    memcpy(dest + dest_len, src, src_len + 1);
  } else {
    // error: buffer overflow
    char* emsg =
        my_sprintf(NULL, "FATAL: buffer overflow at file '%s':%d", file, line);
    logfatal(emsg);
  }

  return dest;
}

static int my_puts(const char* s) {
  int count = 0;

  while (*s) {
    count += (my_putchar(*s) != EOF);
    ++s;
  }

  return count;
}

/* A simple printf that recognizes color codes */
int colorize(const char* fmt, ...) {
  // delegate to my_vsprintf
  va_list ap;

  va_start(ap, fmt);
  char* buf = my_vsprintf(NULL, fmt, ap);
  va_end(ap);

  // write it
  int rc = my_cputs(buf);
  free(buf);  
  return rc;
}

void output_color(char c) {
  switch (c) {
    case '!':
      output(ANSI_RESET);
      break;
    case 'r':
      output(ANSI_FG_RED);
      break;
    case 'R':
      output(ANSI_FG_BOLD_RED);
      break;
    case 'g':
      output(ANSI_FG_GREEN);
      break;
    case 'G':
      output(ANSI_FG_BOLD_GREEN);
      break;
    case 'y':
      output(ANSI_FG_YELLOW);
      break;
    case 'Y':
      output(ANSI_FG_BOLD_YELLOW);
      break;
    case 'b':
      output(ANSI_FG_BLUE);
      break;
    case 'B':
      output(ANSI_FG_BOLD_BLUE);
      break;
    case 'm':
    case 'p':
      output(ANSI_FG_MAGENTA);
      break;
    case 'M':
    case 'P':
      output(ANSI_FG_BOLD_MAGENTA);
      break;
    case 'c':
      output(ANSI_FG_CYAN);
      break;
    case 'C':
      output(ANSI_FG_BOLD_CYAN);
      break;
    case 'w':
      output(ANSI_FG_WHITE);
      break;
    case 'W':
      output(ANSI_FG_BOLD_WHITE);
      break;
    case 'd':
    case 'D':
      output(ANSI_FG_BOLD_BLACK);
      break;
    case 'U':

      break;
    default:

      break;
  }
}

/* check for color codes and \r\n translation.  Return the number of characters
   printed. */
static int my_cputs(const char* s) {
  int count = 0;
  bool underline = false;

  while (*s) {
    if (ansi) {
      if (*s == ' ' || *s == '\n' || *s == '\r') {
        if (underline) {
          underline = false;
          output(ANSI_RESET_UNDERSCORE);
        }
      }
    }
    if (*s == '@') {
      s++;
      if (*s == '@') {
        s++;

        if (strchr("!rRgGyYbBcCmMpPwWDU", *s)) {
          if (ansi) {
            output_color(*s);
            if (*s == 'U') {
              underline = true;
              output(ANSI_UNDERSCORE);
            }
          }
        } else {
          s--;
          count += (my_putchar('@') != EOF);
          count += (my_putchar('@') != EOF);
        }

      } else {
        s--;
        count += (my_putchar(*s) != EOF);
      }
    } else {
      count += (my_putchar(*s) != EOF);
    }
    ++s;
  }

  return count;
}

int output(const char* s) {
  while (*s) {
    my_putchar(*s);
    ++s;
  }
  return 0;
}

int my_putchar(int c) {
#ifdef _SSL
  char newline = '\r';

  if (c == '\n') SSL_write(1, &newline, sizeof(newline));
  SSL_write(1, &c, sizeof(c));
  return (c);

#else
  if (c == '\n') {
    putchar('\r');
  }

  return (putchar(c));
#endif
}

int my_putc(int c, FILE* stream) { return putc(c, stream); }
