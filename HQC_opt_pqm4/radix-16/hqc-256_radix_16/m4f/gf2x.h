#ifndef GF2X_H
#define GF2X_H
/**
 * @file gf2x.h
 * @brief Header file for gf2x.c
 */

#include <stdint.h>

void PQCLEAN_HQC256_CLEAN_vect_mul(uint64_t *o, const uint64_t *v1, const uint64_t *v2);

void radix_16_basemul_asm(uint32_t out[2], uint32_t a, uint32_t b);

#endif
