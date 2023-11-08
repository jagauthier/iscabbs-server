#include <stdbool.h>
#include <stdint.h>

#include "defs.h"
#include "ext.h"

#define S8 sizeof(uint8_t)
#define S16 sizeof(uint16_t)
#define S32 sizeof(uint32_t)

extern bool x86;

unsigned getbits(uint32_t, uint8_t, uint8_t);

struct userdata *get_users(void);
struct user *my_finduser(const char *, uint32_t, int, struct userdata *);

uint8_t convert_bigbtmp(void);
uint8_t convert_msgdata(void);
uint8_t convert_user(void);
uint8_t convert_messages(void);
uint8_t convert_room_descs(void);
uint8_t rebuild_messages(void);
uint8_t convert_mail(void);
int32_t find_magic_location(uint32_t);
uint8_t find_magics(void);