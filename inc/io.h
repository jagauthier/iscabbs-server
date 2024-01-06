#define ANSI_RESET         "\x1b[0m"ANSI_BG_BLACK

// Text formatting
#define ANSI_BOLD          "\x1b[1m"
#define ANSI_UNDERSCORE    "\x1b[4m"
#define ANSI_BLINK         "\x1b[5m"
#define ANSI_REVERSE       "\x1b[7m"
#define ANSI_CONCEAL       "\x1b[8m"

// Foreground colors
#define ANSI_FG_BLACK      ANSI_RESET"\x1b[30m"ANSI_BG_BLACK
#define ANSI_FG_RED        ANSI_RESET"\x1b[31m"ANSI_BG_BLACK
#define ANSI_FG_GREEN      ANSI_RESET"\x1b[32m"ANSI_BG_BLACK
#define ANSI_FG_YELLOW     ANSI_RESET"\x1b[33m"ANSI_BG_BLACK
#define ANSI_FG_BLUE       ANSI_RESET"\x1b[34m"ANSI_BG_BLACK
#define ANSI_FG_MAGENTA    ANSI_RESET"\x1b[35m"ANSI_BG_BLACK
#define ANSI_FG_CYAN       ANSI_RESET"\x1b[36m"ANSI_BG_BLACK
#define ANSI_FG_WHITE      ANSI_RESET"\x1b[37m"ANSI_BG_BLACK

// Foreground colors
#define ANSI_FG_BOLD_BLACK      ANSI_BOLD"\x1b[30m"ANSI_BG_BLACK
#define ANSI_FG_BOLD_RED        ANSI_BOLD"\x1b[31m"ANSI_BG_BLACK
#define ANSI_FG_BOLD_GREEN      ANSI_BOLD"\x1b[32m"ANSI_BG_BLACK
#define ANSI_FG_BOLD_YELLOW     ANSI_BOLD"\x1b[33m"ANSI_BG_BLACK
#define ANSI_FG_BOLD_BLUE       ANSI_BOLD"\x1b[34m"ANSI_BG_BLACK
#define ANSI_FG_BOLD_MAGENTA    ANSI_BOLD"\x1b[35m"ANSI_BG_BLACK
#define ANSI_FG_BOLD_CYAN       ANSI_BOLD"\x1b[36m"ANSI_BG_BLACK
#define ANSI_FG_BOLD_WHITE      ANSI_BOLD"\x1b[37m"ANSI_BG_BLACK

// Background colors
#define ANSI_BG_BLACK      "\x1b[40m"
#define ANSI_BG_RED        "\x1b[41m"
#define ANSI_BG_GREEN      "\x1b[42m"
#define ANSI_BG_YELLOW     "\x1b[43m"
#define ANSI_BG_BLUE       "\x1b[44m"
#define ANSI_BG_MAGENTA    "\x1b[45m"
#define ANSI_BG_CYAN       "\x1b[46m"
#define ANSI_BG_WHITE      "\x1b[47m"

// Reset particular attributes
#define ANSI_RESET_BOLD       "\x1b[21m"
#define ANSI_RESET_DIM        "\x1b[22m"
#define ANSI_RESET_UNDERSCORE "\x1b[24m"
#define ANSI_RESET_BLINK      "\x1b[25m"
#define ANSI_RESET_REVERSE    "\x1b[27m"
#define ANSI_RESET_CONCEAL    "\x1b[28m"

#define BOLD_RED "@@R"
#define BOLD_YELLOW "@@Y"
#define BOLD_GREEN "@@G"
#define BOLD_BLUE "@@B"
#define BOLD_CYAN "@@C"
#define BOLD_MAGENTA "@@M"
#define BOLD_WHITE "@@W"

#define RED "@@r"
#define YELLOW "@@y"
#define GREEN "@@g"
#define BLUE "@@b"
#define CYAN "@@c"
#define MAGENTA "@@m"
#define WHITE "@@w"
#define RESET "@@!"