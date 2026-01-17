#ifndef CRT_FFT_H_
#define CRT_FFT_H_

#include <stdint.h>
void bit_shift_xor_193_asm(uint32_t *res, const uint32_t *A, const uint32_t *idx_mod32_sorted, const uint32_t mod32_cnt[32]);
void CRT_fft_HQC3(uint32_t *res, const uint32_t *A, const uint32_t *B);

#endif