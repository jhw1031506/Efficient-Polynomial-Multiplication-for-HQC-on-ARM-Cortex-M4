#include <string.h>
#include "gfmul_fft.h"
#include "parameters.h"
#include "radix16.h"
#include "reduce.h"
#include "CRT_fft.h"
#include "outstat_index.h"

#define N_FAFFT 32768
#define N_FAFFT_32 1024

#define LOW_LEN 81 
const uint32_t p_inv[LOW_LEN] = {
    0x946e1d09, 0xa96ad483, 0xe3067976, 0x188428c3, 0xbe44b89b, 0xd94c36fa, 0x64fde8cd, 0x06c2edd1,
    0x03f2214a, 0xb99a1ccf, 0x501db63c, 0xcbc29153, 0xe59a87ea, 0xc20fab2e, 0x6a5f1263, 0x644adb71,
    0xacfe3aed, 0x7ea8b8fa, 0xc910187d, 0x56688751, 0x616b724f, 0x63fff975, 0x6f36cee9, 0x681afeb2,
    0x63673a09, 0x2aeab0dc, 0x5daa0ac2, 0x055f4b33, 0xd581c2af, 0xce7cab57, 0x3b427bf7, 0x2606d27f,
    0x5c7733f7, 0x53ebb36d, 0xb38eb677, 0x47840fe2, 0xc9d1186c, 0x88481c32, 0x07ebc33b, 0x9316380c,
    0x3c1185a4, 0xff447307, 0x4bbd9b6a, 0x859a039f, 0xfe461341, 0x415a57ed, 0xdd5e814d, 0x129477a0,
    0x268a81f1, 0x0ca77a62, 0x153f6960, 0x86c32a4d, 0x3def7418, 0x17c4a4d1, 0xe366c874, 0x93cfea0b,
    0x57b16e9a, 0x0afc1068, 0xccf4208d, 0x8ff50caf, 0x065d6f2f, 0x9f97ba45, 0x27b420f9, 0x2bd3c006,
    0xac0e1cda, 0x86494168, 0xb7dfc818, 0x61bbec38, 0x128d7cb6, 0xf108751f, 0x512a92f6, 0xb59b6bfb,
    0x62bd921d, 0x54712067, 0x45e3b4eb, 0x313d9fa7, 0x2ab8de2b, 0x5eec9010, 0xbded9976, 0x38f712bf,
    0xcef335b7};

static void poly_xor(uint32_t *res, uint32_t *A, uint32_t *B, uint32_t len){
	for(uint32_t i=0; i<len; ++i){
		res[i] = A[i] ^ B[i];
	}
}

void CRT_fft_HQC1(uint32_t *res, const uint32_t *A, const uint32_t *B){
    uint32_t A_FAFFT[N_FAFFT_32] = {0}, B_FAFFT[N_FAFFT_32] = {0};
    uint32_t rx[LOW_LEN << 1];
    uint32_t tmp[LOW_LEN];
    uint32_t t[LOW_LEN << 1];
    uint32_t r[OUT_WORDS] = {0};
    memcpy(A_FAFFT, A, VEC_N_SIZE_32 * 4);
    memcpy(B_FAFFT, B, VEC_N_SIZE_32 * 4);
    bmul2_4096_to_4096((uint8_t *)r, (uint8_t *)A_FAFFT, (uint8_t *)B_FAFFT);
    radix16_81(rx, A, B);
    poly_xor(tmp, rx, r, LOW_LEN);
    radix16_81(t, tmp, p_inv);
    bit_shift_xor_81_output_asm(r, t, blk_voff, blk_cnt, NBLOCKS);
    reduce(res, r);
}
