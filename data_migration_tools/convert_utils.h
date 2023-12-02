#include <stdbool.h>
#include <stdint.h>

#include "defs.h"
#include "ext.h"
#include "bbs_new.h"

#define S8 sizeof(uint8_t)
#define S16 sizeof(uint16_t)
#define S32 sizeof(uint32_t)

unsigned getbits(uint32_t, uint8_t, uint8_t);


uint8_t convert_messages(void);
