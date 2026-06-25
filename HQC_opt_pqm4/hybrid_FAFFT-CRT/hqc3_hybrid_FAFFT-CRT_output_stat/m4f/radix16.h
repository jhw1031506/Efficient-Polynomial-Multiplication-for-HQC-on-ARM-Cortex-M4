#ifndef _RADIX16_H_
#define _RADIX16_H_

#include <stdint.h>

void radix_16_basemul_asm(uint32_t out[2], uint32_t a, uint32_t b);

void radix16_193(uint32_t *o, const uint32_t *a, const uint32_t *b);

#endif
