#ifndef CRT_FFT_H_
#define CRT_FFT_H_

#include <stdint.h>

void CRT_fft_HQC1(uint32_t *res, const uint32_t *A, const uint32_t *B);

void bit_shift_xor_81_output_asm(uint32_t *res, const uint32_t *t, const int32_t *voff, const uint32_t *cnt, uint32_t nblocks);

#endif