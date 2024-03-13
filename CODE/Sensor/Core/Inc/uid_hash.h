#ifndef _STM32_UIDHASH_
#define _STM32_UIDHASH_

#include <stdint-gcc.h>
#include <stddef.h>

uint32_t Hash32Len5to12(const char *s, size_t len);

#endif