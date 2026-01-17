// Implemented by Jihoon Jang and Myeonghoon Lee
#include "gf2x.h"
#include "parameters.h"
#include <stddef.h>
#include <stdint.h>
#include "reduce.h"
#include <string.h>

//CASE can be 1-6
#ifndef CASE
    #define CASE 6
#endif

#if CASE==1 
#define WMUL 1
#elif CASE==2
#define WMUL 2
#elif CASE==3
#define WMUL 3
#elif CASE==4 
#define WMUL 4
#elif CASE==5
#define WMUL 5
#elif CASE==6
#define WMUL 6 //default
#endif

static inline void divide_by_x_plus_one(uint32_t *in, uint32_t *out, int32_t size){
    out[0] = in[0];
    for(int32_t i = 1; i < size; i++) {
        out[i] = out[i - 1] ^ in[i];
    }
}

#define SWAPMOVE_CONST(RES, A, mask, n, tmp) \
{ \
    asm volatile( \
    	"EOR.W %0, %1, %1, LSR #"#n"\n" \
	: "=r"(tmp) \
	: "r"(A));\
	tmp &= (mask); \
    RES = A ^ tmp;\
    asm volatile( \
    	"EOR.W %0, %0, %1, LSL #"#n"\n" \
	: "+r"(RES) \
	: "r"(tmp));\
}

#define SWAPMOVE(A, B, mask, n, tmp) \
{ \
    asm volatile( \
    	"EOR.W %0, %1, %2, LSR #"#n"\n" \
	: "=r"(tmp) \
	: "r"(B), "r"(A));\
	tmp &= (mask); \
    B ^= tmp; \
    asm volatile( \
    	"EOR.W %0, %0, %1, LSL #"#n"\n" \
	: "+r"(A) \
	: "r"(tmp));\
}

/*
#define MUL32TO64(A, B, L, H) \
{ \
  __asm__("umull %0, %1, %2, %3" \
          : "=r"(L), "=r"(H) \
          : "r"(A),    "r"(B)); \
}

#define MLA32TO64(A, B, L, H) \
{ \
  __asm__("umlal %0, %1, %2, %3" \
          : "+r"(L), "+r"(H) \
          : "r"(A),    "r"(B)); \
}
void radix_16_basemul(uint32_t out[2], uint32_t a, uint32_t b){

       static const uint32_t mask32 = 0x11111111;
       uint32_t a0, a1, a2, a3, b0, b1, b2, b3;

       a0 = (a)&mask32;
       b0 = (b)&mask32;
   
       uint32_t b1 = ((b)>>2)&mask32;
       uint32_t b2 = ((b)>>1)&mask32;
       uint32_t b3 = ((b)>>3)&mask32;
   
       uint32_t tmp[8]={0};

       MUL32TO64(a0, b0, out[0], tmp[1]);//0
       out[0] &= mask32;
       //tmp[1] &= mask32;

       MUL32TO64(a0, b2, tmp[2], tmp[3]);//1
       //tmp[2] &= mask32;
       //tmp[3] &= mask32;

       MLA32TO64(a0, b1, tmp[1], tmp[2]);//2
       tmp[1] &= mask32;
       //tmp[2] &= mask32;

       uint32_t a1 = ((a)>>2)&mask32;

   	   MLA32TO64(a1, b0, tmp[1], tmp[2]);//3
   	   tmp[1] &= mask32;
   	   tmp[2] &= mask32;

       out[0]=tmp[0]^(tmp[1]<<2);

   	   MUL32TO64(a1, b3, tmp[4], tmp[5]);//4
       //tmp[4] &= mask32;
       //tmp[5] &= mask32;

       MLA32TO64(a0, b3, tmp[3], tmp[4]);//5
       //tmp[3] &= mask32;
       //tmp[4] &= mask32;

       MLA32TO64(a1, b1, tmp[2], tmp[3]);//6
       tmp[2] &= mask32;
       tmp[3] &= mask32;

       MLA32TO64(a1, b2, tmp[3], tmp[4]);//7
       //tmp[3] &= mask32;
       tmp[4] &= mask32;

       uint32_t a2 = ((a)>>1)&mask32;

       MLA32TO64(a2, b0, tmp[2], tmp[3]);//8
       tmp[2] &= mask32;
       tmp[3] &= mask32;

       out[0]=out[0]^(tmp[2]<<1);

       MLA32TO64(a2, b1, tmp[3], tmp[4]);//9
       tmp[3] &= mask32;
       //tmp[4] &= mask32;

       MLA32TO64(a2, b2, tmp[4], tmp[5]);//10
       tmp[4] &= mask32;
       //tmp[5] &= mask32;

       uint32_t a3 = ((a)>>3)&mask32;

       MLA32TO64(a3, b0, tmp[3], tmp[4]);//11
       tmp[3] &= mask32;
       //tmp[4] &= mask32;

       out[0]=out[0]^(tmp[3]<<3);

       MUL32TO64(a3, b3, tmp[6], tmp[7]);//12
       //tmp[6] &= mask32;
       tmp[7] &= mask32;

       MLA32TO64(a2, b3, tmp[5], tmp[6]);//13
       tmp[5] &= mask32;
       //tmp[6] &= mask32;

       MLA32TO64(a3, b1, tmp[4], tmp[5]);//14
       tmp[4] &= mask32;
       //tmp[5] &= mask32;

       MLA32TO64(a3, b2, tmp[5], tmp[6]);//15
       tmp[5] &= mask32;
       tmp[6] &= mask32;

       out[1]=tmp[4]^(tmp[5]<<2);
       out[1]=out[1]^(tmp[6]<<1);
       out[1]=out[1]^(tmp[7]<<3);
}
*/

static inline void swap32(uint32_t *res, const uint32_t *x) {
    uint32_t tmp;
	SWAPMOVE_CONST(res[0], x[0], 0x0000F0F0u, 12, tmp);
	SWAPMOVE(res[0], res[0], 0x00cc00ccu, 6, tmp);
	SWAPMOVE(res[0], res[0], 0x0a0a0a0au, 3, tmp);
}
static inline void poly_swap(uint32_t *res, const uint32_t * x, uint32_t len) {
	for(uint32_t i=0;i<len;i++){
		swap32(&res[i], &x[i]);
	}
}

static inline void inv_swap32(uint32_t * x) {
    uint32_t tmp;
	SWAPMOVE(x[0], x[0], 0x0a0a0a0au, 3, tmp);
	SWAPMOVE(x[0], x[0], 0x00cc00ccu, 6, tmp);
	SWAPMOVE(x[0], x[0], 0x0000F0F0u, 12, tmp);
}
static inline void poly_invswap(uint32_t * x, uint32_t len) {
	for(uint32_t i=0;i<len;i++){
		inv_swap32(&x[i]);
	}
}

static inline
void mul2(uint32_t *c, const uint32_t *a, const uint32_t *b)
{
   uint32_t hs0, hs1;
   uint32_t hl2[2];

   hs0 = a[0] ^ a[1];
   hs1 = b[0] ^ b[1];

   radix_16_basemul_asm(c, a[0], b[0]);
   radix_16_basemul_asm(c+2, a[1], b[1]);
   radix_16_basemul_asm(hl2, hs0, hs1);


   hl2[0] = hl2[0] ^ c[0] ^ c[2];
   hl2[1] = hl2[1] ^ c[1] ^ c[3];

   c[1] ^= hl2[0];
   c[2] ^= hl2[1];
}


/*
 * This version of mul3 I got from Weimerskirch, Stebila,
 * and Shantz, "Generic GF(2^m) arithmetic in software
 * an its application to ECC" (ACISP 2003).
 */
#define gfmul_3 mul3
static inline
void mul3 (uint32_t *c, const uint32_t *a, const uint32_t *b)
{
   uint32_t d0[2], d1[2], d2[2], d01[2], d02[2], d12[2], middle;

   radix_16_basemul_asm(d0, a[0], b[0]);
   radix_16_basemul_asm(d1, a[1], b[1]);
   radix_16_basemul_asm(d2, a[2], b[2]);
   radix_16_basemul_asm(d01, a[0]^a[1], b[0]^b[1]);
   radix_16_basemul_asm(d02, a[0]^a[2], b[0]^b[2]);
   radix_16_basemul_asm(d12, a[1]^a[2], b[1]^b[2]);

    middle = d0[1] ^ d1[1] ^ d1[0] ^ d2[0];
    c[0] = d0[0];
    c[1] = d0[1] ^ d01[0] ^ d1[0] ^ d0[0];
    c[2] = d01[1] ^ d02[0] ^ d0[0] ^ middle;
    c[3] = d02[1] ^ d2[1] ^ d12[0] ^ middle;
    c[4] = d12[1] ^ d1[1] ^ d2[1] ^ d2[0];
    c[5] = d2[1];
}
#define gfmul_4 mul4
static inline
void mul4(uint32_t *c, const uint32_t *a, const uint32_t *b)
{
   uint32_t hs0[2], hs1[2];
   uint32_t hl2[4];

   hs0[0] = a[0] ^ a[2];
   hs0[1] = a[1] ^ a[3];
   hs1[0] = b[0] ^ b[2];
   hs1[1] = b[1] ^ b[3];

   mul2(c, a, b);
   mul2(c+4, a+2, b+2);
   mul2(hl2, hs0, hs1);

   hl2[0] = hl2[0] ^ c[0] ^ c[4];
   hl2[1] = hl2[1] ^ c[1] ^ c[5];
   hl2[2] = hl2[2] ^ c[2] ^ c[6];
   hl2[3] = hl2[3] ^ c[3] ^ c[7];

   c[2] ^= hl2[0];
   c[3] ^= hl2[1];
   c[4] ^= hl2[2];
   c[5] ^= hl2[3];
}

#define gfmul_5 mul5
static inline
void mul5 (uint32_t *c, const uint32_t *a, const uint32_t *b)
{
   uint32_t hs0[3], hs1[3];
   uint32_t hl2[6];

   hs0[0] = a[0] ^ a[3];
   hs0[1] = a[1] ^ a[4];
   hs0[2] = a[2];
   hs1[0] = b[0] ^ b[3];
   hs1[1] = b[1] ^ b[4];
   hs1[2] = b[2];

   mul3(c, a, b);
   mul3(hl2, hs0, hs1);
   mul2(c+6, a+3, b+3);

   hl2[0] = hl2[0] ^ c[0] ^ c[6];
   hl2[1] = hl2[1] ^ c[1] ^ c[7];
   hl2[2] = hl2[2] ^ c[2] ^ c[8];
   hl2[3] = hl2[3] ^ c[3] ^ c[9];
   hl2[4] = hl2[4] ^ c[4];
   hl2[5] = hl2[5] ^ c[5];


   c[3] ^= hl2[0];
   c[4] ^= hl2[1];
   c[5] ^= hl2[2];
   c[6] ^= hl2[3];
   c[7] ^= hl2[4];
   c[8] ^= hl2[5];

}

#define gfmul_6 mul6
static inline
void mul6(uint32_t *c, const uint32_t *a, const uint32_t *b)
{
   uint32_t hs0[3], hs1[3];
   uint32_t hl2[6];

   hs0[0] = a[0] ^ a[3];
   hs0[1] = a[1] ^ a[4];
   hs0[2] = a[2] ^ a[5];
   hs1[0] = b[0] ^ b[3];
   hs1[1] = b[1] ^ b[4];
   hs1[2] = b[2] ^ b[5];

   mul3(c, a, b);
   mul3(c+6, a+3, b+3);
   mul3(hl2, hs0, hs1);

   hl2[0] = hl2[0] ^ c[0] ^ c[6];
   hl2[1] = hl2[1] ^ c[1] ^ c[7];
   hl2[2] = hl2[2] ^ c[2] ^ c[8];
   hl2[3] = hl2[3] ^ c[3] ^ c[9];
   hl2[4] = hl2[4] ^ c[4] ^ c[10];
   hl2[5] = hl2[5] ^ c[5] ^ c[11];

   c[3] ^= hl2[0];
   c[4] ^= hl2[1];
   c[5] ^= hl2[2];
   c[6] ^= hl2[3];
   c[7] ^= hl2[4];
   c[8] ^= hl2[5];
}

#define gfmul_7 mul7
static inline
void mul7(uint32_t *c, const uint32_t *a, const uint32_t *b)
{
   uint32_t hs0[4], hs1[4];
   uint32_t hl2[8];

   hs0[0] = a[0] ^ a[4];
   hs0[1] = a[1] ^ a[5];
   hs0[2] = a[2] ^ a[6];
   hs0[3] = a[3];
   hs1[0] = b[0] ^ b[4];
   hs1[1] = b[1] ^ b[5];
   hs1[2] = b[2] ^ b[6];
   hs1[3] = b[3];

   mul4(c, a, b);
   mul4(hl2, hs0, hs1);
   mul3(c+8, a+4, b+4);

   hl2[0] = hl2[0] ^ c[0] ^ c[8];
   hl2[1] = hl2[1] ^ c[1] ^ c[9];
   hl2[2] = hl2[2] ^ c[2] ^ c[10];
   hl2[3] = hl2[3] ^ c[3] ^ c[11];
   hl2[4] = hl2[4] ^ c[4] ^ c[12];
   hl2[5] = hl2[5] ^ c[5] ^ c[13];
   hl2[6] = hl2[6] ^ c[6];
   hl2[7] = hl2[7] ^ c[7];

   c[4]  ^= hl2[0];
   c[5]  ^= hl2[1];
   c[6]  ^= hl2[2];
   c[7]  ^= hl2[3];
   c[8]  ^= hl2[4];
   c[9]  ^= hl2[5];
   c[10] ^= hl2[6];
   c[11] ^= hl2[7];

}

#define gfmul_8 mul8
static inline
void mul8(uint32_t *c, const uint32_t *a, const uint32_t *b)
{
   uint32_t hs0[4], hs1[4];
   uint32_t hl2[8];

   hs0[0] = a[0] ^ a[4];
   hs0[1] = a[1] ^ a[5];
   hs0[2] = a[2] ^ a[6];
   hs0[3] = a[3] ^ a[7];
   hs1[0] = b[0] ^ b[4];
   hs1[1] = b[1] ^ b[5];
   hs1[2] = b[2] ^ b[6];
   hs1[3] = b[3] ^ b[7];

   mul4(c, a, b);
   mul4(c+8, a+4, b+4);
   mul4(hl2, hs0, hs1);

   hl2[0] = hl2[0] ^ c[0] ^ c[8];
   hl2[1] = hl2[1] ^ c[1] ^ c[9];
   hl2[2] = hl2[2] ^ c[2] ^ c[10];
   hl2[3] = hl2[3] ^ c[3] ^ c[11];
   hl2[4] = hl2[4] ^ c[4] ^ c[12];
   hl2[5] = hl2[5] ^ c[5] ^ c[13];
   hl2[6] = hl2[6] ^ c[6] ^ c[14];
   hl2[7] = hl2[7] ^ c[7] ^ c[15];

   c[4]  ^= hl2[0];
   c[5]  ^= hl2[1];
   c[6]  ^= hl2[2];
   c[7]  ^= hl2[3];
   c[8]  ^= hl2[4];
   c[9]  ^= hl2[5];
   c[10] ^= hl2[6];
   c[11] ^= hl2[7];

}
/*
#if WMUL == 4 || WMUL == 5 || WMUL == 6
#define gfmul_12 karat2_12_my
static inline void karat2_12_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[12], D1[12], D2[12], SAA[6], SBB[6];
    gfmul_6(D0, A, B);
    gfmul_6(D2, (A+6), (B+6));
    for(int32_t i = 0; i < 6; i++) {
        int32_t is = i + 6;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
    gfmul_6(D1, SAA, SBB);
    for(int32_t i = 0; i < 6; i++) {
        int32_t is = i + 6;
        int32_t is2 = is + 6;
        int32_t is3 = is2 + 6;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
}
#endif

#if WMUL== 2 || WMUL == 3 || WMUL == 4 || WMUL == 5 || WMUL == 6
#define gfmul_13 karat2_13_my
static inline void karat2_13_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[14], D1[14], D2[12], SAA[7], SBB[7];
    gfmul_7(D0, A, B);
    gfmul_6(D2, (A+7), (B+7));
    for(int32_t i = 0; i < 6; i++) {
        int32_t is = i + 7;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
        SAA[6]=A[6];
        SBB[6]=B[6];
    gfmul_7(D1, SAA, SBB);
    for(int32_t i = 0; i < 5; i++) {
        int32_t is = i + 7;
        int32_t is2 = is + 7;
        int32_t is3 = is2 + 7;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
    for(int32_t i = 5; i < 7; i++) {
        int32_t is = i + 7;
        int32_t is2 = is + 7;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is];
    }
}
#endif

#if WMUL== 2 || WMUL == 3 || WMUL == 4 || WMUL == 5 || WMUL == 6
#define gfmul_14 karat2_14_my
static inline void karat2_14_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[14], D1[14], D2[14], SAA[7], SBB[7];
    gfmul_7(D0, A, B);
    gfmul_7(D2, (A+7), (B+7));
    for(int32_t i = 0; i < 7; i++) {
        int32_t is = i + 7;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
    gfmul_7(D1, SAA, SBB);
    for(int32_t i = 0; i < 7; i++) {
        int32_t is = i + 7;
        int32_t is2 = is + 7;
        int32_t is3 = is2 + 7;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
}
#endif

#define gfmul_15 karat2_15_my
static inline void karat2_15_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[16], D1[16], D2[14], SAA[8], SBB[8];
    gfmul_8(D0, A, B);
    gfmul_7(D2, (A+8), (B+8));
    for(int32_t i = 0; i < 7; i++) {
        int32_t is = i + 8;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
        SAA[7]=A[7];
        SBB[7]=B[7];
    gfmul_8(D1, SAA, SBB);
    for(int32_t i = 0; i < 6; i++) {
        int32_t is = i + 8;
        int32_t is2 = is + 8;
        int32_t is3 = is2 + 8;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
    for(int32_t i = 6; i < 8; i++) {
        int32_t is = i + 8;
        int32_t is2 = is + 8;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is];
    }
}

#define gfmul_16 karat2_16_my
static inline void karat2_16_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[16], D1[16], D2[16], SAA[8], SBB[8];
    gfmul_8(D0, A, B);
    gfmul_8(D2, (A+8), (B+8));
    for(int32_t i = 0; i < 8; i++) {
        int32_t is = i + 8;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
    gfmul_8(D1, SAA, SBB);
    for(int32_t i = 0; i < 8; i++) {
        int32_t is = i + 8;
        int32_t is2 = is + 8;
        int32_t is3 = is2 + 8;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
}

#if WMUL == 1
#define gfmul_30 karat2_30_my
static inline void karat2_30_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[30], D1[30], D2[30], SAA[15], SBB[15];
    gfmul_15(D0, A, B);
    gfmul_15(D2, (A+15), (B+15));
    for(int32_t i = 0; i < 15; i++) {
        int32_t is = i + 15;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
    gfmul_15(D1, SAA, SBB);
    for(int32_t i = 0; i < 15; i++) {
        int32_t is = i + 15;
        int32_t is2 = is + 15;
        int32_t is3 = is2 + 15;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
}
#endif

#if WMUL == 1 ||  WMUL== 2 || WMUL == 3 || WMUL == 4 || WMUL == 5
#define gfmul_31 karat2_31_my
static inline void karat2_31_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[32], D1[32], D2[30], SAA[16], SBB[16];
    gfmul_16(D0, A, B);
    gfmul_15(D2, (A+16), (B+16));
    for(int32_t i = 0; i < 15; i++) {
        int32_t is = i + 16;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
        SAA[15]=A[15];
        SBB[15]=B[15];
    gfmul_16(D1, SAA, SBB);
    for(int32_t i = 0; i < 14; i++) {
        int32_t is = i + 16;
        int32_t is2 = is + 16;
        int32_t is3 = is2 + 16;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
    for(int32_t i = 14; i < 16; i++) {
        int32_t is = i + 16;
        int32_t is2 = is + 16;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is];
    }
}
#endif

#if WMUL == 1 ||  WMUL== 2 || WMUL == 3 || WMUL == 4 || WMUL == 5
#define gfmul_32 karat2_32_my
static inline void karat2_32_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[32], D1[32], D2[32], SAA[16], SBB[16];
    gfmul_16(D0, A, B);
    gfmul_16(D2, (A+16), (B+16));
    for(int32_t i = 0; i < 16; i++) {
        int32_t is = i + 16;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
    gfmul_16(D1, SAA, SBB);
    for(int32_t i = 0; i < 16; i++) {
        int32_t is = i + 16;
        int32_t is2 = is + 16;
        int32_t is3 = is2 + 16;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
}
#endif

#if WMUL == 4 || WMUL == 5 || WMUL == 6
#define gfmul_40 tc3_40_my
static inline void tc3_40_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[28], W1[29], W2[30], W3[30], W4[24], tmp[30];
    U0 = &A[0];
    U1 = &A[14];
    U2 = &A[28];
    V0 = &B[0];
    V1 = &B[14];
    V2 = &B[28];
    for (int32_t i = 0 ; i < 12 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[12] = U0[12] ^ U1[12];
    W2[12] = V0[12] ^ V1[12];
    W3[13] = U0[13] ^ U1[13];
    W2[13] = V0[13] ^ V1[13];
    gfmul_14(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 13 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    
    W0[14] = U1[13];
    W4[14] = V1[13];
    for (int32_t i = 0 ; i < 14 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[14] = W0[14];
    W2[14] = W4[14];
    for (int32_t i = 0 ; i < 14 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_15(tmp, W3, W2);
    for (int32_t i = 0 ; i < 30 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_15(W2, W0, W4);
    gfmul_12(W4, U2, V2);
    gfmul_14(W0, U0, V0);
    for (int32_t i = 0 ; i < 30 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 28 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 27 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[27] = W2[28];
    W2[28] = W2[29];
    for (int32_t i = 0 ; i < 24 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 24 ; i < 29 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[29] = W3[29];
    for (int32_t i = 0 ; i < 24 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 30);
    for (int32_t i = 0 ; i < 27 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[27] = W3[28];
    tmp[28] = W3[29];
    divide_by_x_plus_one(tmp, W3, 29);
    for (int32_t i = 0 ; i < 24 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 24 ; i < 28 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[28] = W2[28];
    for (int32_t i = 0 ; i < 28 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 10; i++) {
        int32_t j = i + 14;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 14] = W1[j] ^ W2[i];
        Out[j + 28] = W2[j] ^ W3[i];
        Out[i + 56] = W3[j] ^ W4[i];
        Out[j + 56] = W4[j];
    }
    for (int32_t i = 10; i < 14; i++) {
        int32_t j = i + 14;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 14] = W1[j] ^ W2[i];
        Out[j + 28] = W2[j] ^ W3[i];
        Out[i + 56] = W3[j] ^ W4[i];
    }
    Out[42] ^= W1[28];
    Out[56] ^= W2[28];
}
#endif

#if WMUL== 2 || WMUL == 3 || WMUL == 4 || WMUL == 5 || WMUL == 6
#define gfmul_41 tc3_41_my
static inline void tc3_41_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
        static uint32_t W0[28], W1[29], W2[30], W3[30], W4[26], tmp[30];
    U0 = &A[0];
    U1 = &A[14];
    U2 = &A[28];
    V0 = &B[0];
    V1 = &B[14];
    V2 = &B[28];
    for (int32_t i = 0 ; i < 13 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[13] = U0[13] ^ U1[13];
    W2[13] = V0[13] ^ V1[13];
    gfmul_14(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 14 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    for (int32_t i = 0 ; i < 14 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[14] = W0[14];
    W2[14] = W4[14];
    for (int32_t i = 0 ; i < 14 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_15(tmp, W3, W2);
    for (int32_t i = 0 ; i < 30 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_15(W2, W0, W4);
    gfmul_13(W4, U2, V2);
    gfmul_14(W0, U0, V0);
    for (int32_t i = 0 ; i < 30 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 28 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 27 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[27] = W2[28];
    W2[28] = W2[29];
    for (int32_t i = 0 ; i < 26 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 26 ; i < 29 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[29] = W3[29];
    for (int32_t i = 0 ; i < 26 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 30);
    for (int32_t i = 0 ; i < 27 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[27] = W3[28];
    tmp[28] = W3[29];
    divide_by_x_plus_one(tmp, W3, 29);
    for (int32_t i = 0 ; i < 26 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 26 ; i < 28 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[28] = W2[28];
    for (int32_t i = 0 ; i < 28 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 12; i++) {
        int32_t j = i + 14;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 14] = W1[j] ^ W2[i];
        Out[j + 28] = W2[j] ^ W3[i];
        Out[i + 56] = W3[j] ^ W4[i];
        Out[j + 56] = W4[j];
    }
    for (int32_t i = 12; i < 14; i++) {
        int32_t j = i + 14;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 14] = W1[j] ^ W2[i];
        Out[j + 28] = W2[j] ^ W3[i];
        Out[i + 56] = W3[j] ^ W4[i];
    }
    Out[42] ^= W1[28];
    Out[56] ^= W2[28];
}
#endif

#if WMUL == 3 || WMUL == 4 || WMUL == 5 || WMUL == 6
#define gfmul_42 tc3_42_my
static inline void tc3_42_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[28], W1[31], W2[32], W3[32], W4[28], tmp[32];
    U0 = &A[0];
    U1 = &A[14];
    U2 = &A[28];
    V0 = &B[0];
    V1 = &B[14];
    V2 = &B[28];
    for (int32_t i = 0 ; i < 14 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    gfmul_14(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 14 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    W0[15] = U2[13];
    W4[15] = V2[13];
    for (int32_t i = 0 ; i < 14 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[14] = W0[14];
    W3[15] = W0[15];
    W2[14] = W4[14];
    W2[15] = W4[15];
    for (int32_t i = 0 ; i < 14 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_16(tmp, W3, W2);
    for (int32_t i = 0 ; i < 32 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_16(W2, W0, W4);
    gfmul_14(W4, U2, V2);
    gfmul_14(W0, U0, V0);
    for (int32_t i = 0 ; i < 32 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 28 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 27 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[27] = W2[28];
    W2[28] = W2[29];
    W2[29] = W2[30];
    W2[30] = W2[31];
    for (int32_t i = 0 ; i < 28 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 28 ; i < 31 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[31] = W3[31];
    for (int32_t i = 0 ; i < 28 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 32);
    for (int32_t i = 0 ; i < 27 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[27] = W3[28];
    tmp[28] = W3[29];
    tmp[29] = W3[30];
    tmp[30] = W3[31];
    divide_by_x_plus_one(tmp, W3, 31);
    for (int32_t i = 0 ; i < 28 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 28 ; i < 31 ; i++) {
        W1[i] = W2[i];
    }
    for (int32_t i = 0 ; i < 30 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 14; i++) {
        int32_t j = i + 14;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 14] = W1[j] ^ W2[i];
        Out[j + 28] = W2[j] ^ W3[i];
        Out[i + 56] = W3[j] ^ W4[i];
        Out[j + 56] = W4[j];
    }
    Out[42] ^= W1[28];
    Out[43] ^= W1[29];
    Out[44] ^= W1[30];
    Out[56] ^= W2[28];
    Out[57] ^= W2[29];
    Out[58] ^= W2[30];
    Out[70] ^= W3[28];
    Out[71] ^= W3[29];
}
#endif


#if WMUL== 2 || WMUL == 3 || WMUL == 4 || WMUL == 5 || WMUL == 6
#define gfmul_43 tc3_43_my
static inline void tc3_43_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[30], W1[31], W2[32], W3[32], W4[26], tmp[32];
    U0 = &A[0];
    U1 = &A[15];
    U2 = &A[30];
    V0 = &B[0];
    V1 = &B[15];
    V2 = &B[30];
    for (int32_t i = 0 ; i < 13 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[13] = U0[13] ^ U1[13];
    W2[13] = V0[13] ^ V1[13];
    W3[14] = U0[14] ^ U1[14];
    W2[14] = V0[14] ^ V1[14];
    gfmul_15(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 14 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    
    W0[15] = U1[14];
    W4[15] = V1[14];
    for (int32_t i = 0 ; i < 15 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[15] = W0[15];
    W2[15] = W4[15];
    for (int32_t i = 0 ; i < 15 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_16(tmp, W3, W2);
    for (int32_t i = 0 ; i < 32 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_16(W2, W0, W4);
    gfmul_13(W4, U2, V2);
    gfmul_15(W0, U0, V0);
    for (int32_t i = 0 ; i < 32 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 30 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 29 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[29] = W2[30];
    W2[30] = W2[31];
    for (int32_t i = 0 ; i < 26 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 26 ; i < 31 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[31] = W3[31];
    for (int32_t i = 0 ; i < 26 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 32);
    for (int32_t i = 0 ; i < 29 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[29] = W3[30];
    tmp[30] = W3[31];
    divide_by_x_plus_one(tmp, W3, 31);
    for (int32_t i = 0 ; i < 26 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 26 ; i < 30 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[30] = W2[30];
    for (int32_t i = 0 ; i < 30 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 11; i++) {
        int32_t j = i + 15;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 15] = W1[j] ^ W2[i];
        Out[j + 30] = W2[j] ^ W3[i];
        Out[i + 60] = W3[j] ^ W4[i];
        Out[j + 60] = W4[j];
    }
    for (int32_t i = 11; i < 15; i++) {
        int32_t j = i + 15;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 15] = W1[j] ^ W2[i];
        Out[j + 30] = W2[j] ^ W3[i];
        Out[i + 60] = W3[j] ^ W4[i];
    }
    Out[45] ^= W1[30];
    Out[60] ^= W2[30];
}
#endif

#if WMUL== 5 || WMUL == 6
#define gfmul_44 tc3_44_my
static inline void tc3_44_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
        static uint32_t W0[30], W1[31], W2[32], W3[32], W4[28], tmp[32];
    U0 = &A[0];
    U1 = &A[15];
    U2 = &A[30];
    V0 = &B[0];
    V1 = &B[15];
    V2 = &B[30];
    for (int32_t i = 0 ; i < 14 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[14] = U0[14] ^ U1[14];
    W2[14] = V0[14] ^ V1[14];
    gfmul_15(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 15 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    for (int32_t i = 0 ; i < 15 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[15] = W0[15];
    W2[15] = W4[15];
    for (int32_t i = 0 ; i < 15 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_16(tmp, W3, W2);
    for (int32_t i = 0 ; i < 32 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_16(W2, W0, W4);
    gfmul_14(W4, U2, V2);
    gfmul_15(W0, U0, V0);
    for (int32_t i = 0 ; i < 32 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 30 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 29 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[29] = W2[30];
    W2[30] = W2[31];
    for (int32_t i = 0 ; i < 28 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 28 ; i < 31 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[31] = W3[31];
    for (int32_t i = 0 ; i < 28 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 32);
    for (int32_t i = 0 ; i < 29 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[29] = W3[30];
    tmp[30] = W3[31];
    divide_by_x_plus_one(tmp, W3, 31);
    for (int32_t i = 0 ; i < 28 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 28 ; i < 30 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[30] = W2[30];
    for (int32_t i = 0 ; i < 30 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 13; i++) {
        int32_t j = i + 15;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 15] = W1[j] ^ W2[i];
        Out[j + 30] = W2[j] ^ W3[i];
        Out[i + 60] = W3[j] ^ W4[i];
        Out[j + 60] = W4[j];
    }
    for (int32_t i = 13; i < 15; i++) {
        int32_t j = i + 15;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 15] = W1[j] ^ W2[i];
        Out[j + 30] = W2[j] ^ W3[i];
        Out[i + 60] = W3[j] ^ W4[i];
    }
    Out[45] ^= W1[30];
    Out[60] ^= W2[30];
}
#endif

#if WMUL== 1
#define gfmul_61 karat2_61_my
static inline void karat2_61_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[62], D1[62], D2[60], SAA[31], SBB[31];
    gfmul_31(D0, A, B);
    gfmul_30(D2, (A+31), (B+31));
    for(int32_t i = 0; i < 30; i++) {
        int32_t is = i + 31;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
        SAA[30]=A[30];
        SBB[30]=B[30];
    gfmul_31(D1, SAA, SBB);
    for(int32_t i = 0; i < 29; i++) {
        int32_t is = i + 31;
        int32_t is2 = is + 31;
        int32_t is3 = is2 + 31;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
    for(int32_t i = 29; i < 31; i++) {
        int32_t is = i + 31;
        int32_t is2 = is + 31;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is];
    }
}
#endif

#if WMUL== 1 || WMUL == 2 || WMUL == 3 
#define gfmul_62 karat2_62_my
static inline void karat2_62_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[62], D1[62], D2[62], SAA[31], SBB[31];
    gfmul_31(D0, A, B);
    gfmul_31(D2, (A+31), (B+31));
    for(int32_t i = 0; i < 31; i++) {
        int32_t is = i + 31;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
    gfmul_31(D1, SAA, SBB);
    for(int32_t i = 0; i < 31; i++) {
        int32_t is = i + 31;
        int32_t is2 = is + 31;
        int32_t is3 = is2 + 31;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
}
#endif

#if WMUL== 1 || WMUL == 2 || WMUL == 3 || WMUL == 4 || WMUL == 5
#define gfmul_63 karat2_63_my
static inline void karat2_63_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[64], D1[64], D2[62], SAA[32], SBB[32];
    gfmul_32(D0, A, B);
    gfmul_31(D2, (A+32), (B+32));
    for(int32_t i = 0; i < 31; i++) {
        int32_t is = i + 32;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
        SAA[31]=A[31];
        SBB[31]=B[31];
    gfmul_32(D1, SAA, SBB);
    for(int32_t i = 0; i < 30; i++) {
        int32_t is = i + 32;
        int32_t is2 = is + 32;
        int32_t is3 = is2 + 32;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
    for(int32_t i = 30; i < 32; i++) {
        int32_t is = i + 32;
        int32_t is2 = is + 32;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is];
    }
}
#endif 

#if WMUL== 1 || WMUL == 2 || WMUL == 3 || WMUL == 4
#define gfmul_64 karat2_64_my
static inline void karat2_64_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[64], D1[64], D2[64], SAA[32], SBB[32];
    gfmul_32(D0, A, B);
    gfmul_32(D2, (A+32), (B+32));
    for(int32_t i = 0; i < 32; i++) {
        int32_t is = i + 32;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
    gfmul_32(D1, SAA, SBB);
    for(int32_t i = 0; i < 32; i++) {
        int32_t is = i + 32;
        int32_t is2 = is + 32;
        int32_t is3 = is2 + 32;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
}
#endif 

#if WMUL ==1
#define gfmul_123 karat2_123_my
static inline void karat2_123_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[124], D1[124], D2[122], SAA[62], SBB[62];
    gfmul_62(D0, A, B);
    gfmul_61(D2, (A+62), (B+62));
    for(int32_t i = 0; i < 61; i++) {
        int32_t is = i + 62;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
        SAA[61]=A[61];
        SBB[61]=B[61];
    gfmul_62(D1, SAA, SBB);
    for(int32_t i = 0; i < 60; i++) {
        int32_t is = i + 62;
        int32_t is2 = is + 62;
        int32_t is3 = is2 + 62;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
    for(int32_t i = 60; i < 62; i++) {
        int32_t is = i + 62;
        int32_t is2 = is + 62;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is];
    }
}
#else
#define gfmul_123 tc3_123_my
static inline void tc3_123_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[82], W1[85], W2[86], W3[86], W4[82], tmp[86];
    U0 = &A[0];
    U1 = &A[41];
    U2 = &A[82];
    V0 = &B[0];
    V1 = &B[41];
    V2 = &B[82];
    for (int32_t i = 0 ; i < 41 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    gfmul_41(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 41 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    W0[42] = U2[40];
    W4[42] = V2[40];
    for (int32_t i = 0 ; i < 41 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[41] = W0[41];
    W3[42] = W0[42];
    W2[41] = W4[41];
    W2[42] = W4[42];
    for (int32_t i = 0 ; i < 41 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_43(tmp, W3, W2);
    for (int32_t i = 0 ; i < 86 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_43(W2, W0, W4);
    gfmul_41(W4, U2, V2);
    gfmul_41(W0, U0, V0);
    for (int32_t i = 0 ; i < 86 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 82 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 81 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[81] = W2[82];
    W2[82] = W2[83];
    W2[83] = W2[84];
    W2[84] = W2[85];
    for (int32_t i = 0 ; i < 82 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 82 ; i < 85 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[85] = W3[85];
    for (int32_t i = 0 ; i < 82 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 86);
    for (int32_t i = 0 ; i < 81 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[81] = W3[82];
    tmp[82] = W3[83];
    tmp[83] = W3[84];
    tmp[84] = W3[85];
    divide_by_x_plus_one(tmp, W3, 85);
    for (int32_t i = 0 ; i < 82 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 82 ; i < 85 ; i++) {
        W1[i] = W2[i];
    }
    for (int32_t i = 0 ; i < 84 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 41; i++) {
        int32_t j = i + 41;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 41] = W1[j] ^ W2[i];
        Out[j + 82] = W2[j] ^ W3[i];
        Out[i + 164] = W3[j] ^ W4[i];
        Out[j + 164] = W4[j];
    }
    Out[123] ^= W1[82];
    Out[124] ^= W1[83];
    Out[125] ^= W1[84];
    Out[164] ^= W2[82];
    Out[165] ^= W2[83];
    Out[166] ^= W2[84];
    Out[205] ^= W3[82];
    Out[206] ^= W3[83];
}
#endif

#if WMUL== 1 || WMUL == 2 || WMUL == 3
#define gfmul_124 karat2_124_my
static inline void karat2_124_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[124], D1[124], D2[124], SAA[62], SBB[62];
    gfmul_62(D0, A, B);
    gfmul_62(D2, (A+62), (B+62));
    for(int32_t i = 0; i < 62; i++) {
        int32_t is = i + 62;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
    gfmul_62(D1, SAA, SBB);
    for(int32_t i = 0; i < 62; i++) {
        int32_t is = i + 62;
        int32_t is2 = is + 62;
        int32_t is3 = is2 + 62;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
}
#else
#define gfmul_124 tc3_124_my
static inline void tc3_124_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[84], W1[85], W2[86], W3[86], W4[80], tmp[86];
    U0 = &A[0];
    U1 = &A[42];
    U2 = &A[84];
    V0 = &B[0];
    V1 = &B[42];
    V2 = &B[84];
    for (int32_t i = 0 ; i < 40 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[40] = U0[40] ^ U1[40];
    W2[40] = V0[40] ^ V1[40];
    W3[41] = U0[41] ^ U1[41];
    W2[41] = V0[41] ^ V1[41];
    gfmul_42(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 41 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    
    W0[42] = U1[41];
    W4[42] = V1[41];
    for (int32_t i = 0 ; i < 42 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[42] = W0[42];
    W2[42] = W4[42];
    for (int32_t i = 0 ; i < 42 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_43(tmp, W3, W2);
    for (int32_t i = 0 ; i < 86 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_43(W2, W0, W4);
    gfmul_40(W4, U2, V2);
    gfmul_42(W0, U0, V0);
    for (int32_t i = 0 ; i < 86 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 84 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 83 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[83] = W2[84];
    W2[84] = W2[85];
    for (int32_t i = 0 ; i < 80 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 80 ; i < 85 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[85] = W3[85];
    for (int32_t i = 0 ; i < 80 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 86);
    for (int32_t i = 0 ; i < 83 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[83] = W3[84];
    tmp[84] = W3[85];
    divide_by_x_plus_one(tmp, W3, 85);
    for (int32_t i = 0 ; i < 80 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 80 ; i < 84 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[84] = W2[84];
    for (int32_t i = 0 ; i < 84 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 38; i++) {
        int32_t j = i + 42;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 42] = W1[j] ^ W2[i];
        Out[j + 84] = W2[j] ^ W3[i];
        Out[i + 168] = W3[j] ^ W4[i];
        Out[j + 168] = W4[j];
    }
    for (int32_t i = 38; i < 42; i++) {
        int32_t j = i + 42;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 42] = W1[j] ^ W2[i];
        Out[j + 84] = W2[j] ^ W3[i];
        Out[i + 168] = W3[j] ^ W4[i];
    }
    Out[126] ^= W1[84];
    Out[168] ^= W2[84];
}
#endif


#if WMUL== 1 || WMUL == 2
#define gfmul_125 karat2_125_my
static inline void karat2_125_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[126], D1[126], D2[124], SAA[63], SBB[63];
    gfmul_63(D0, A, B);
    gfmul_62(D2, (A+63), (B+63));
    for(int32_t i = 0; i < 62; i++) {
        int32_t is = i + 63;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
        SAA[62]=A[62];
        SBB[62]=B[62];
    gfmul_63(D1, SAA, SBB);
    for(int32_t i = 0; i < 61; i++) {
        int32_t is = i + 63;
        int32_t is2 = is + 63;
        int32_t is3 = is2 + 63;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
    for(int32_t i = 61; i < 63; i++) {
        int32_t is = i + 63;
        int32_t is2 = is + 63;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is];
    }
}

#else

#define gfmul_125 tc3_125_my
static inline void tc3_125_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
        static uint32_t W0[84], W1[85], W2[86], W3[86], W4[82], tmp[86];
    U0 = &A[0];
    U1 = &A[42];
    U2 = &A[84];
    V0 = &B[0];
    V1 = &B[42];
    V2 = &B[84];
    for (int32_t i = 0 ; i < 41 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[41] = U0[41] ^ U1[41];
    W2[41] = V0[41] ^ V1[41];
    gfmul_42(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 42 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    for (int32_t i = 0 ; i < 42 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[42] = W0[42];
    W2[42] = W4[42];
    for (int32_t i = 0 ; i < 42 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_43(tmp, W3, W2);
    for (int32_t i = 0 ; i < 86 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_43(W2, W0, W4);
    gfmul_41(W4, U2, V2);
    gfmul_42(W0, U0, V0);
    for (int32_t i = 0 ; i < 86 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 84 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 83 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[83] = W2[84];
    W2[84] = W2[85];
    for (int32_t i = 0 ; i < 82 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 82 ; i < 85 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[85] = W3[85];
    for (int32_t i = 0 ; i < 82 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 86);
    for (int32_t i = 0 ; i < 83 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[83] = W3[84];
    tmp[84] = W3[85];
    divide_by_x_plus_one(tmp, W3, 85);
    for (int32_t i = 0 ; i < 82 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 82 ; i < 84 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[84] = W2[84];
    for (int32_t i = 0 ; i < 84 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 40; i++) {
        int32_t j = i + 42;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 42] = W1[j] ^ W2[i];
        Out[j + 84] = W2[j] ^ W3[i];
        Out[i + 168] = W3[j] ^ W4[i];
        Out[j + 168] = W4[j];
    }
    for (int32_t i = 40; i < 42; i++) {
        int32_t j = i + 42;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 42] = W1[j] ^ W2[i];
        Out[j + 84] = W2[j] ^ W3[i];
        Out[i + 168] = W3[j] ^ W4[i];
    }
    Out[126] ^= W1[84];
    Out[168] ^= W2[84];
}
#endif

#if WMUL== 1 || WMUL == 2 || WMUL == 3 || WMUL == 4 || WMUL == 5
#define gfmul_126 karat2_126_my
static inline void karat2_126_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[126], D1[126], D2[126], SAA[63], SBB[63];
    gfmul_63(D0, A, B);
    gfmul_63(D2, (A+63), (B+63));
    for(int32_t i = 0; i < 63; i++) {
        int32_t is = i + 63;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
    gfmul_63(D1, SAA, SBB);
    for(int32_t i = 0; i < 63; i++) {
        int32_t is = i + 63;
        int32_t is2 = is + 63;
        int32_t is3 = is2 + 63;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
}
#else
#define gfmul_126 tc3_126_my
static inline void tc3_126_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[84], W1[87], W2[88], W3[88], W4[84], tmp[88];
    U0 = &A[0];
    U1 = &A[42];
    U2 = &A[84];
    V0 = &B[0];
    V1 = &B[42];
    V2 = &B[84];
    for (int32_t i = 0 ; i < 42 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    gfmul_42(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 42 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    W0[43] = U2[41];
    W4[43] = V2[41];
    for (int32_t i = 0 ; i < 42 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[42] = W0[42];
    W3[43] = W0[43];
    W2[42] = W4[42];
    W2[43] = W4[43];
    for (int32_t i = 0 ; i < 42 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_44(tmp, W3, W2);
    for (int32_t i = 0 ; i < 88 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_44(W2, W0, W4);
    gfmul_42(W4, U2, V2);
    gfmul_42(W0, U0, V0);
    for (int32_t i = 0 ; i < 88 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 84 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 83 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[83] = W2[84];
    W2[84] = W2[85];
    W2[85] = W2[86];
    W2[86] = W2[87];
    for (int32_t i = 0 ; i < 84 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 84 ; i < 87 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[87] = W3[87];
    for (int32_t i = 0 ; i < 84 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 88);
    for (int32_t i = 0 ; i < 83 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[83] = W3[84];
    tmp[84] = W3[85];
    tmp[85] = W3[86];
    tmp[86] = W3[87];
    divide_by_x_plus_one(tmp, W3, 87);
    for (int32_t i = 0 ; i < 84 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 84 ; i < 87 ; i++) {
        W1[i] = W2[i];
    }
    for (int32_t i = 0 ; i < 86 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 42; i++) {
        int32_t j = i + 42;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 42] = W1[j] ^ W2[i];
        Out[j + 84] = W2[j] ^ W3[i];
        Out[i + 168] = W3[j] ^ W4[i];
        Out[j + 168] = W4[j];
    }
    Out[126] ^= W1[84];
    Out[127] ^= W1[85];
    Out[128] ^= W1[86];
    Out[168] ^= W2[84];
    Out[169] ^= W2[85];
    Out[170] ^= W2[86];
    Out[210] ^= W3[84];
    Out[211] ^= W3[85];
}
#endif


#if WMUL== 1 || WMUL == 2 || WMUL == 3 || WMUL == 4
#define gfmul_127 karat2_127_my
static inline void karat2_127_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[128], D1[128], D2[126], SAA[64], SBB[64];
    gfmul_64(D0, A, B);
    gfmul_63(D2, (A+64), (B+64));
    for(int32_t i = 0; i < 63; i++) {
        int32_t is = i + 64;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
        SAA[63]=A[63];
        SBB[63]=B[63];
    gfmul_64(D1, SAA, SBB);
    for(int32_t i = 0; i < 62; i++) {
        int32_t is = i + 64;
        int32_t is2 = is + 64;
        int32_t is3 = is2 + 64;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
    for(int32_t i = 62; i < 64; i++) {
        int32_t is = i + 64;
        int32_t is2 = is + 64;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is];
    }
}

#else

#define gfmul_127 tc3_127_my
static inline void tc3_127_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[86], W1[87], W2[88], W3[88], W4[82], tmp[88];
    U0 = &A[0];
    U1 = &A[43];
    U2 = &A[86];
    V0 = &B[0];
    V1 = &B[43];
    V2 = &B[86];
    for (int32_t i = 0 ; i < 41 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[41] = U0[41] ^ U1[41];
    W2[41] = V0[41] ^ V1[41];
    W3[42] = U0[42] ^ U1[42];
    W2[42] = V0[42] ^ V1[42];
    gfmul_43(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 42 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    
    W0[43] = U1[42];
    W4[43] = V1[42];
    for (int32_t i = 0 ; i < 43 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[43] = W0[43];
    W2[43] = W4[43];
    for (int32_t i = 0 ; i < 43 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_44(tmp, W3, W2);
    for (int32_t i = 0 ; i < 88 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_44(W2, W0, W4);
    gfmul_41(W4, U2, V2);
    gfmul_43(W0, U0, V0);
    for (int32_t i = 0 ; i < 88 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 86 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 85 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[85] = W2[86];
    W2[86] = W2[87];
    for (int32_t i = 0 ; i < 82 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 82 ; i < 87 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[87] = W3[87];
    for (int32_t i = 0 ; i < 82 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 88);
    for (int32_t i = 0 ; i < 85 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[85] = W3[86];
    tmp[86] = W3[87];
    divide_by_x_plus_one(tmp, W3, 87);
    for (int32_t i = 0 ; i < 82 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 82 ; i < 86 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[86] = W2[86];
    for (int32_t i = 0 ; i < 86 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 39; i++) {
        int32_t j = i + 43;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 43] = W1[j] ^ W2[i];
        Out[j + 86] = W2[j] ^ W3[i];
        Out[i + 172] = W3[j] ^ W4[i];
        Out[j + 172] = W4[j];
    }
    for (int32_t i = 39; i < 43; i++) {
        int32_t j = i + 43;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 43] = W1[j] ^ W2[i];
        Out[j + 86] = W2[j] ^ W3[i];
        Out[i + 172] = W3[j] ^ W4[i];
    }
    Out[129] ^= W1[86];
    Out[172] ^= W2[86];
}
#endif

#define gfmul_373 tc3_373_my
static inline void tc3_373_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[250], W1[251], W2[252], W3[252], W4[246], tmp[252];
    U0 = &A[0];
    U1 = &A[125];
    U2 = &A[250];
    V0 = &B[0];
    V1 = &B[125];
    V2 = &B[250];
    for (int32_t i = 0 ; i < 123 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[123] = U0[123] ^ U1[123];
    W2[123] = V0[123] ^ V1[123];
    W3[124] = U0[124] ^ U1[124];
    W2[124] = V0[124] ^ V1[124];
    gfmul_125(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 124 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    
    W0[125] = U1[124];
    W4[125] = V1[124];
    for (int32_t i = 0 ; i < 125 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[125] = W0[125];
    W2[125] = W4[125];
    for (int32_t i = 0 ; i < 125 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_126(tmp, W3, W2);
    for (int32_t i = 0 ; i < 252 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_126(W2, W0, W4);
    gfmul_123(W4, U2, V2);
    gfmul_125(W0, U0, V0);
    for (int32_t i = 0 ; i < 252 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 250 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 249 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[249] = W2[250];
    W2[250] = W2[251];
    for (int32_t i = 0 ; i < 246 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 246 ; i < 251 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[251] = W3[251];
    for (int32_t i = 0 ; i < 246 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 252);
    for (int32_t i = 0 ; i < 249 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[249] = W3[250];
    tmp[250] = W3[251];
    divide_by_x_plus_one(tmp, W3, 251);
    for (int32_t i = 0 ; i < 246 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 246 ; i < 250 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[250] = W2[250];
    for (int32_t i = 0 ; i < 250 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 121; i++) {
        int32_t j = i + 125;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 125] = W1[j] ^ W2[i];
        Out[j + 250] = W2[j] ^ W3[i];
        Out[i + 500] = W3[j] ^ W4[i];
        Out[j + 500] = W4[j];
    }
    for (int32_t i = 121; i < 125; i++) {
        int32_t j = i + 125;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 125] = W1[j] ^ W2[i];
        Out[j + 250] = W2[j] ^ W3[i];
        Out[i + 500] = W3[j] ^ W4[i];
    }
    Out[375] ^= W1[250];
    Out[500] ^= W2[250];
}

#define gfmul_374 tc3_374_my
static inline void tc3_374_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
        static uint32_t W0[250], W1[251], W2[252], W3[252], W4[248], tmp[252];
    U0 = &A[0];
    U1 = &A[125];
    U2 = &A[250];
    V0 = &B[0];
    V1 = &B[125];
    V2 = &B[250];
    for (int32_t i = 0 ; i < 124 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[124] = U0[124] ^ U1[124];
    W2[124] = V0[124] ^ V1[124];
    gfmul_125(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 125 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    for (int32_t i = 0 ; i < 125 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[125] = W0[125];
    W2[125] = W4[125];
    for (int32_t i = 0 ; i < 125 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_126(tmp, W3, W2);
    for (int32_t i = 0 ; i < 252 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_126(W2, W0, W4);
    gfmul_124(W4, U2, V2);
    gfmul_125(W0, U0, V0);
    for (int32_t i = 0 ; i < 252 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 250 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 249 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[249] = W2[250];
    W2[250] = W2[251];
    for (int32_t i = 0 ; i < 248 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 248 ; i < 251 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[251] = W3[251];
    for (int32_t i = 0 ; i < 248 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 252);
    for (int32_t i = 0 ; i < 249 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[249] = W3[250];
    tmp[250] = W3[251];
    divide_by_x_plus_one(tmp, W3, 251);
    for (int32_t i = 0 ; i < 248 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 248 ; i < 250 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[250] = W2[250];
    for (int32_t i = 0 ; i < 250 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 123; i++) {
        int32_t j = i + 125;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 125] = W1[j] ^ W2[i];
        Out[j + 250] = W2[j] ^ W3[i];
        Out[i + 500] = W3[j] ^ W4[i];
        Out[j + 500] = W4[j];
    }
    for (int32_t i = 123; i < 125; i++) {
        int32_t j = i + 125;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 125] = W1[j] ^ W2[i];
        Out[j + 250] = W2[j] ^ W3[i];
        Out[i + 500] = W3[j] ^ W4[i];
    }
    Out[375] ^= W1[250];
    Out[500] ^= W2[250];
}

#define gfmul_375 tc3_375_my
static inline void tc3_375_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[250], W1[253], W2[254], W3[254], W4[250], tmp[254];
    U0 = &A[0];
    U1 = &A[125];
    U2 = &A[250];
    V0 = &B[0];
    V1 = &B[125];
    V2 = &B[250];
    for (int32_t i = 0 ; i < 125 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    gfmul_125(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 125 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    W0[126] = U2[124];
    W4[126] = V2[124];
    for (int32_t i = 0 ; i < 125 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[125] = W0[125];
    W3[126] = W0[126];
    W2[125] = W4[125];
    W2[126] = W4[126];
    for (int32_t i = 0 ; i < 125 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_127(tmp, W3, W2);
    for (int32_t i = 0 ; i < 254 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_127(W2, W0, W4);
    gfmul_125(W4, U2, V2);
    gfmul_125(W0, U0, V0);
    for (int32_t i = 0 ; i < 254 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 250 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 249 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[249] = W2[250];
    W2[250] = W2[251];
    W2[251] = W2[252];
    W2[252] = W2[253];
    for (int32_t i = 0 ; i < 250 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 250 ; i < 253 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[253] = W3[253];
    for (int32_t i = 0 ; i < 250 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 254);
    for (int32_t i = 0 ; i < 249 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[249] = W3[250];
    tmp[250] = W3[251];
    tmp[251] = W3[252];
    tmp[252] = W3[253];
    divide_by_x_plus_one(tmp, W3, 253);
    for (int32_t i = 0 ; i < 250 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 250 ; i < 253 ; i++) {
        W1[i] = W2[i];
    }
    for (int32_t i = 0 ; i < 252 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 125; i++) {
        int32_t j = i + 125;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 125] = W1[j] ^ W2[i];
        Out[j + 250] = W2[j] ^ W3[i];
        Out[i + 500] = W3[j] ^ W4[i];
        Out[j + 500] = W4[j];
    }
    Out[375] ^= W1[250];
    Out[376] ^= W1[251];
    Out[377] ^= W1[252];
    Out[500] ^= W2[250];
    Out[501] ^= W2[251];
    Out[502] ^= W2[252];
    Out[625] ^= W3[250];
    Out[626] ^= W3[251];
}

#define gfmul_1121 tc3_1121_my
static inline void tc3_1121_my(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
        static uint32_t W0[748], W1[749], W2[750], W3[750], W4[746], tmp[750];
    U0 = &A[0];
    U1 = &A[374];
    U2 = &A[748];
    V0 = &B[0];
    V1 = &B[374];
    V2 = &B[748];
    for (int32_t i = 0 ; i < 373 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[373] = U0[373] ^ U1[373];
    W2[373] = V0[373] ^ V1[373];
    gfmul_374(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 374 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    for (int32_t i = 0 ; i < 374 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[374] = W0[374];
    W2[374] = W4[374];
    for (int32_t i = 0 ; i < 374 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_375(tmp, W3, W2);
    for (int32_t i = 0 ; i < 750 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_375(W2, W0, W4);
    gfmul_373(W4, U2, V2);
    gfmul_374(W0, U0, V0);
    for (int32_t i = 0 ; i < 750 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 748 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 747 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[747] = W2[748];
    W2[748] = W2[749];
    for (int32_t i = 0 ; i < 746 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 746 ; i < 749 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[749] = W3[749];
    for (int32_t i = 0 ; i < 746 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 750);
    for (int32_t i = 0 ; i < 747 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[747] = W3[748];
    tmp[748] = W3[749];
    divide_by_x_plus_one(tmp, W3, 749);
    for (int32_t i = 0 ; i < 746 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 746 ; i < 748 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[748] = W2[748];
    for (int32_t i = 0 ; i < 748 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 372; i++) {
        int32_t j = i + 374;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 374] = W1[j] ^ W2[i];
        Out[j + 748] = W2[j] ^ W3[i];
        Out[i + 1496] = W3[j] ^ W4[i];
        Out[j + 1496] = W4[j];
    }
    for (int32_t i = 372; i < 374; i++) {
        int32_t j = i + 374;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 374] = W1[j] ^ W2[i];
        Out[j + 748] = W2[j] ^ W3[i];
        Out[i + 1496] = W3[j] ^ W4[i];
    }
    Out[1122] ^= W1[748];
    Out[1496] ^= W2[748];
}

*/

#define gfmul_12 karat2_12
static inline void karat2_12(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[12], D1[12], D2[12], SAA[6], SBB[6];
    gfmul_6(D0, A, B);
    gfmul_6(D2, (A+6), (B+6));
    for(int32_t i = 0; i < 6; i++) {
        int32_t is = i + 6;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
    gfmul_6(D1, SAA, SBB);
    for(int32_t i = 0; i < 6; i++) {
        int32_t is = i + 6;
        int32_t is2 = is + 6;
        int32_t is3 = is2 + 6;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
}

#define gfmul_13 karat2_13
static inline void karat2_13(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[14], D1[14], D2[12], SAA[7], SBB[7];
    gfmul_7(D0, A, B);
    gfmul_6(D2, (A+7), (B+7));
    for(int32_t i = 0; i < 6; i++) {
        int32_t is = i + 7;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
        SAA[6]=A[6];
        SBB[6]=B[6];
    gfmul_7(D1, SAA, SBB);
    for(int32_t i = 0; i < 5; i++) {
        int32_t is = i + 7;
        int32_t is2 = is + 7;
        int32_t is3 = is2 + 7;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
    for(int32_t i = 5; i < 7; i++) {
        int32_t is = i + 7;
        int32_t is2 = is + 7;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is];
    }
}

#define gfmul_14 karat2_14
static inline void karat2_14(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[14], D1[14], D2[14], SAA[7], SBB[7];
    gfmul_7(D0, A, B);
    gfmul_7(D2, (A+7), (B+7));
    for(int32_t i = 0; i < 7; i++) {
        int32_t is = i + 7;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
    gfmul_7(D1, SAA, SBB);
    for(int32_t i = 0; i < 7; i++) {
        int32_t is = i + 7;
        int32_t is2 = is + 7;
        int32_t is3 = is2 + 7;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
}

#define gfmul_15 karat2_15
static inline void karat2_15(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[16], D1[16], D2[14], SAA[8], SBB[8];
    gfmul_8(D0, A, B);
    gfmul_7(D2, (A+8), (B+8));
    for(int32_t i = 0; i < 7; i++) {
        int32_t is = i + 8;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
        SAA[7]=A[7];
        SBB[7]=B[7];
    gfmul_8(D1, SAA, SBB);
    for(int32_t i = 0; i < 6; i++) {
        int32_t is = i + 8;
        int32_t is2 = is + 8;
        int32_t is3 = is2 + 8;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
    for(int32_t i = 6; i < 8; i++) {
        int32_t is = i + 8;
        int32_t is2 = is + 8;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is];
    }
}

#define gfmul_16 karat2_16
static inline void karat2_16(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[16], D1[16], D2[16], SAA[8], SBB[8];
    gfmul_8(D0, A, B);
    gfmul_8(D2, (A+8), (B+8));
    for(int32_t i = 0; i < 8; i++) {
        int32_t is = i + 8;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
    gfmul_8(D1, SAA, SBB);
    for(int32_t i = 0; i < 8; i++) {
        int32_t is = i + 8;
        int32_t is2 = is + 8;
        int32_t is3 = is2 + 8;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
}

#define gfmul_40 tc3_40
static inline void tc3_40(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[28], W1[29], W2[30], W3[30], W4[24], tmp[30];
    U0 = &A[0];
    U1 = &A[14];
    U2 = &A[28];
    V0 = &B[0];
    V1 = &B[14];
    V2 = &B[28];
    for (int32_t i = 0 ; i < 12 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[12] = U0[12] ^ U1[12];
    W2[12] = V0[12] ^ V1[12];
    W3[13] = U0[13] ^ U1[13];
    W2[13] = V0[13] ^ V1[13];
    gfmul_14(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 13 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    
    W0[14] = U1[13];
    W4[14] = V1[13];
    for (int32_t i = 0 ; i < 14 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[14] = W0[14];
    W2[14] = W4[14];
    for (int32_t i = 0 ; i < 14 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_15(tmp, W3, W2);
    for (int32_t i = 0 ; i < 30 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_15(W2, W0, W4);
    gfmul_12(W4, U2, V2);
    gfmul_14(W0, U0, V0);
    for (int32_t i = 0 ; i < 30 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 28 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 27 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[27] = W2[28];
    W2[28] = W2[29];
    for (int32_t i = 0 ; i < 24 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 24 ; i < 29 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[29] = W3[29];
    for (int32_t i = 0 ; i < 24 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 30);
    for (int32_t i = 0 ; i < 27 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[27] = W3[28];
    tmp[28] = W3[29];
    divide_by_x_plus_one(tmp, W3, 29);
    for (int32_t i = 0 ; i < 24 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 24 ; i < 28 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[28] = W2[28];
    for (int32_t i = 0 ; i < 28 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 10; i++) {
        int32_t j = i + 14;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 14] = W1[j] ^ W2[i];
        Out[j + 28] = W2[j] ^ W3[i];
        Out[i + 56] = W3[j] ^ W4[i];
        Out[j + 56] = W4[j];
    }
    for (int32_t i = 10; i < 14; i++) {
        int32_t j = i + 14;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 14] = W1[j] ^ W2[i];
        Out[j + 28] = W2[j] ^ W3[i];
        Out[i + 56] = W3[j] ^ W4[i];
    }
    Out[42] ^= W1[28];
    Out[56] ^= W2[28];
}

#define gfmul_41 tc3_41
static inline void tc3_41(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
        static uint32_t W0[28], W1[29], W2[30], W3[30], W4[26], tmp[30];
    U0 = &A[0];
    U1 = &A[14];
    U2 = &A[28];
    V0 = &B[0];
    V1 = &B[14];
    V2 = &B[28];
    for (int32_t i = 0 ; i < 13 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[13] = U0[13] ^ U1[13];
    W2[13] = V0[13] ^ V1[13];
    gfmul_14(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 14 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    for (int32_t i = 0 ; i < 14 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[14] = W0[14];
    W2[14] = W4[14];
    for (int32_t i = 0 ; i < 14 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_15(tmp, W3, W2);
    for (int32_t i = 0 ; i < 30 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_15(W2, W0, W4);
    gfmul_13(W4, U2, V2);
    gfmul_14(W0, U0, V0);
    for (int32_t i = 0 ; i < 30 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 28 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 27 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[27] = W2[28];
    W2[28] = W2[29];
    for (int32_t i = 0 ; i < 26 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 26 ; i < 29 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[29] = W3[29];
    for (int32_t i = 0 ; i < 26 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 30);
    for (int32_t i = 0 ; i < 27 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[27] = W3[28];
    tmp[28] = W3[29];
    divide_by_x_plus_one(tmp, W3, 29);
    for (int32_t i = 0 ; i < 26 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 26 ; i < 28 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[28] = W2[28];
    for (int32_t i = 0 ; i < 28 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 12; i++) {
        int32_t j = i + 14;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 14] = W1[j] ^ W2[i];
        Out[j + 28] = W2[j] ^ W3[i];
        Out[i + 56] = W3[j] ^ W4[i];
        Out[j + 56] = W4[j];
    }
    for (int32_t i = 12; i < 14; i++) {
        int32_t j = i + 14;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 14] = W1[j] ^ W2[i];
        Out[j + 28] = W2[j] ^ W3[i];
        Out[i + 56] = W3[j] ^ W4[i];
    }
    Out[42] ^= W1[28];
    Out[56] ^= W2[28];
}

#define gfmul_42 tc3_42
static inline void tc3_42(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[28], W1[31], W2[32], W3[32], W4[28], tmp[32];
    U0 = &A[0];
    U1 = &A[14];
    U2 = &A[28];
    V0 = &B[0];
    V1 = &B[14];
    V2 = &B[28];
    for (int32_t i = 0 ; i < 14 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    gfmul_14(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 14 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    W0[15] = U2[13];
    W4[15] = V2[13];
    for (int32_t i = 0 ; i < 14 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[14] = W0[14];
    W3[15] = W0[15];
    W2[14] = W4[14];
    W2[15] = W4[15];
    for (int32_t i = 0 ; i < 14 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_16(tmp, W3, W2);
    for (int32_t i = 0 ; i < 32 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_16(W2, W0, W4);
    gfmul_14(W4, U2, V2);
    gfmul_14(W0, U0, V0);
    for (int32_t i = 0 ; i < 32 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 28 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 27 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[27] = W2[28];
    W2[28] = W2[29];
    W2[29] = W2[30];
    W2[30] = W2[31];
    for (int32_t i = 0 ; i < 28 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 28 ; i < 31 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[31] = W3[31];
    for (int32_t i = 0 ; i < 28 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 32);
    for (int32_t i = 0 ; i < 27 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[27] = W3[28];
    tmp[28] = W3[29];
    tmp[29] = W3[30];
    tmp[30] = W3[31];
    divide_by_x_plus_one(tmp, W3, 31);
    for (int32_t i = 0 ; i < 28 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 28 ; i < 31 ; i++) {
        W1[i] = W2[i];
    }
    for (int32_t i = 0 ; i < 30 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 14; i++) {
        int32_t j = i + 14;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 14] = W1[j] ^ W2[i];
        Out[j + 28] = W2[j] ^ W3[i];
        Out[i + 56] = W3[j] ^ W4[i];
        Out[j + 56] = W4[j];
    }
    Out[42] ^= W1[28];
    Out[43] ^= W1[29];
    Out[44] ^= W1[30];
    Out[56] ^= W2[28];
    Out[57] ^= W2[29];
    Out[58] ^= W2[30];
    Out[70] ^= W3[28];
    Out[71] ^= W3[29];
}

#define gfmul_43 tc3_43
static inline void tc3_43(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[30], W1[31], W2[32], W3[32], W4[26], tmp[32];
    U0 = &A[0];
    U1 = &A[15];
    U2 = &A[30];
    V0 = &B[0];
    V1 = &B[15];
    V2 = &B[30];
    for (int32_t i = 0 ; i < 13 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[13] = U0[13] ^ U1[13];
    W2[13] = V0[13] ^ V1[13];
    W3[14] = U0[14] ^ U1[14];
    W2[14] = V0[14] ^ V1[14];
    gfmul_15(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 14 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    
    W0[15] = U1[14];
    W4[15] = V1[14];
    for (int32_t i = 0 ; i < 15 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[15] = W0[15];
    W2[15] = W4[15];
    for (int32_t i = 0 ; i < 15 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_16(tmp, W3, W2);
    for (int32_t i = 0 ; i < 32 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_16(W2, W0, W4);
    gfmul_13(W4, U2, V2);
    gfmul_15(W0, U0, V0);
    for (int32_t i = 0 ; i < 32 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 30 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 29 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[29] = W2[30];
    W2[30] = W2[31];
    for (int32_t i = 0 ; i < 26 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 26 ; i < 31 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[31] = W3[31];
    for (int32_t i = 0 ; i < 26 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 32);
    for (int32_t i = 0 ; i < 29 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[29] = W3[30];
    tmp[30] = W3[31];
    divide_by_x_plus_one(tmp, W3, 31);
    for (int32_t i = 0 ; i < 26 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 26 ; i < 30 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[30] = W2[30];
    for (int32_t i = 0 ; i < 30 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 11; i++) {
        int32_t j = i + 15;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 15] = W1[j] ^ W2[i];
        Out[j + 30] = W2[j] ^ W3[i];
        Out[i + 60] = W3[j] ^ W4[i];
        Out[j + 60] = W4[j];
    }
    for (int32_t i = 11; i < 15; i++) {
        int32_t j = i + 15;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 15] = W1[j] ^ W2[i];
        Out[j + 30] = W2[j] ^ W3[i];
        Out[i + 60] = W3[j] ^ W4[i];
    }
    Out[45] ^= W1[30];
    Out[60] ^= W2[30];
}

#define gfmul_44 tc3_44
static inline void tc3_44(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
        static uint32_t W0[30], W1[31], W2[32], W3[32], W4[28], tmp[32];
    U0 = &A[0];
    U1 = &A[15];
    U2 = &A[30];
    V0 = &B[0];
    V1 = &B[15];
    V2 = &B[30];
    for (int32_t i = 0 ; i < 14 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[14] = U0[14] ^ U1[14];
    W2[14] = V0[14] ^ V1[14];
    gfmul_15(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 15 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    for (int32_t i = 0 ; i < 15 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[15] = W0[15];
    W2[15] = W4[15];
    for (int32_t i = 0 ; i < 15 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_16(tmp, W3, W2);
    for (int32_t i = 0 ; i < 32 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_16(W2, W0, W4);
    gfmul_14(W4, U2, V2);
    gfmul_15(W0, U0, V0);
    for (int32_t i = 0 ; i < 32 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 30 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 29 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[29] = W2[30];
    W2[30] = W2[31];
    for (int32_t i = 0 ; i < 28 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 28 ; i < 31 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[31] = W3[31];
    for (int32_t i = 0 ; i < 28 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 32);
    for (int32_t i = 0 ; i < 29 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[29] = W3[30];
    tmp[30] = W3[31];
    divide_by_x_plus_one(tmp, W3, 31);
    for (int32_t i = 0 ; i < 28 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 28 ; i < 30 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[30] = W2[30];
    for (int32_t i = 0 ; i < 30 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 13; i++) {
        int32_t j = i + 15;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 15] = W1[j] ^ W2[i];
        Out[j + 30] = W2[j] ^ W3[i];
        Out[i + 60] = W3[j] ^ W4[i];
        Out[j + 60] = W4[j];
    }
    for (int32_t i = 13; i < 15; i++) {
        int32_t j = i + 15;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 15] = W1[j] ^ W2[i];
        Out[j + 30] = W2[j] ^ W3[i];
        Out[i + 60] = W3[j] ^ W4[i];
    }
    Out[45] ^= W1[30];
    Out[60] ^= W2[30];
}

#define gfmul_123 tc3_123
static inline void tc3_123(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[82], W1[85], W2[86], W3[86], W4[82], tmp[86];
    U0 = &A[0];
    U1 = &A[41];
    U2 = &A[82];
    V0 = &B[0];
    V1 = &B[41];
    V2 = &B[82];
    for (int32_t i = 0 ; i < 41 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    gfmul_41(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 41 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    W0[42] = U2[40];
    W4[42] = V2[40];
    for (int32_t i = 0 ; i < 41 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[41] = W0[41];
    W3[42] = W0[42];
    W2[41] = W4[41];
    W2[42] = W4[42];
    for (int32_t i = 0 ; i < 41 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_43(tmp, W3, W2);
    for (int32_t i = 0 ; i < 86 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_43(W2, W0, W4);
    gfmul_41(W4, U2, V2);
    gfmul_41(W0, U0, V0);
    for (int32_t i = 0 ; i < 86 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 82 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 81 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[81] = W2[82];
    W2[82] = W2[83];
    W2[83] = W2[84];
    W2[84] = W2[85];
    for (int32_t i = 0 ; i < 82 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 82 ; i < 85 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[85] = W3[85];
    for (int32_t i = 0 ; i < 82 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 86);
    for (int32_t i = 0 ; i < 81 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[81] = W3[82];
    tmp[82] = W3[83];
    tmp[83] = W3[84];
    tmp[84] = W3[85];
    divide_by_x_plus_one(tmp, W3, 85);
    for (int32_t i = 0 ; i < 82 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 82 ; i < 85 ; i++) {
        W1[i] = W2[i];
    }
    for (int32_t i = 0 ; i < 84 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 41; i++) {
        int32_t j = i + 41;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 41] = W1[j] ^ W2[i];
        Out[j + 82] = W2[j] ^ W3[i];
        Out[i + 164] = W3[j] ^ W4[i];
        Out[j + 164] = W4[j];
    }
    Out[123] ^= W1[82];
    Out[124] ^= W1[83];
    Out[125] ^= W1[84];
    Out[164] ^= W2[82];
    Out[165] ^= W2[83];
    Out[166] ^= W2[84];
    Out[205] ^= W3[82];
    Out[206] ^= W3[83];
}

#define gfmul_124 tc3_124
static inline void tc3_124(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[84], W1[85], W2[86], W3[86], W4[80], tmp[86];
    U0 = &A[0];
    U1 = &A[42];
    U2 = &A[84];
    V0 = &B[0];
    V1 = &B[42];
    V2 = &B[84];
    for (int32_t i = 0 ; i < 40 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[40] = U0[40] ^ U1[40];
    W2[40] = V0[40] ^ V1[40];
    W3[41] = U0[41] ^ U1[41];
    W2[41] = V0[41] ^ V1[41];
    gfmul_42(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 41 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    
    W0[42] = U1[41];
    W4[42] = V1[41];
    for (int32_t i = 0 ; i < 42 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[42] = W0[42];
    W2[42] = W4[42];
    for (int32_t i = 0 ; i < 42 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_43(tmp, W3, W2);
    for (int32_t i = 0 ; i < 86 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_43(W2, W0, W4);
    gfmul_40(W4, U2, V2);
    gfmul_42(W0, U0, V0);
    for (int32_t i = 0 ; i < 86 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 84 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 83 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[83] = W2[84];
    W2[84] = W2[85];
    for (int32_t i = 0 ; i < 80 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 80 ; i < 85 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[85] = W3[85];
    for (int32_t i = 0 ; i < 80 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 86);
    for (int32_t i = 0 ; i < 83 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[83] = W3[84];
    tmp[84] = W3[85];
    divide_by_x_plus_one(tmp, W3, 85);
    for (int32_t i = 0 ; i < 80 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 80 ; i < 84 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[84] = W2[84];
    for (int32_t i = 0 ; i < 84 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 38; i++) {
        int32_t j = i + 42;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 42] = W1[j] ^ W2[i];
        Out[j + 84] = W2[j] ^ W3[i];
        Out[i + 168] = W3[j] ^ W4[i];
        Out[j + 168] = W4[j];
    }
    for (int32_t i = 38; i < 42; i++) {
        int32_t j = i + 42;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 42] = W1[j] ^ W2[i];
        Out[j + 84] = W2[j] ^ W3[i];
        Out[i + 168] = W3[j] ^ W4[i];
    }
    Out[126] ^= W1[84];
    Out[168] ^= W2[84];
}

#define gfmul_125 tc3_125
static inline void tc3_125(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
        static uint32_t W0[84], W1[85], W2[86], W3[86], W4[82], tmp[86];
    U0 = &A[0];
    U1 = &A[42];
    U2 = &A[84];
    V0 = &B[0];
    V1 = &B[42];
    V2 = &B[84];
    for (int32_t i = 0 ; i < 41 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[41] = U0[41] ^ U1[41];
    W2[41] = V0[41] ^ V1[41];
    gfmul_42(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 42 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    for (int32_t i = 0 ; i < 42 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[42] = W0[42];
    W2[42] = W4[42];
    for (int32_t i = 0 ; i < 42 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_43(tmp, W3, W2);
    for (int32_t i = 0 ; i < 86 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_43(W2, W0, W4);
    gfmul_41(W4, U2, V2);
    gfmul_42(W0, U0, V0);
    for (int32_t i = 0 ; i < 86 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 84 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 83 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[83] = W2[84];
    W2[84] = W2[85];
    for (int32_t i = 0 ; i < 82 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 82 ; i < 85 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[85] = W3[85];
    for (int32_t i = 0 ; i < 82 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 86);
    for (int32_t i = 0 ; i < 83 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[83] = W3[84];
    tmp[84] = W3[85];
    divide_by_x_plus_one(tmp, W3, 85);
    for (int32_t i = 0 ; i < 82 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 82 ; i < 84 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[84] = W2[84];
    for (int32_t i = 0 ; i < 84 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 40; i++) {
        int32_t j = i + 42;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 42] = W1[j] ^ W2[i];
        Out[j + 84] = W2[j] ^ W3[i];
        Out[i + 168] = W3[j] ^ W4[i];
        Out[j + 168] = W4[j];
    }
    for (int32_t i = 40; i < 42; i++) {
        int32_t j = i + 42;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 42] = W1[j] ^ W2[i];
        Out[j + 84] = W2[j] ^ W3[i];
        Out[i + 168] = W3[j] ^ W4[i];
    }
    Out[126] ^= W1[84];
    Out[168] ^= W2[84];
}

#define gfmul_126 tc3_126
static inline void tc3_126(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[84], W1[87], W2[88], W3[88], W4[84], tmp[88];
    U0 = &A[0];
    U1 = &A[42];
    U2 = &A[84];
    V0 = &B[0];
    V1 = &B[42];
    V2 = &B[84];
    for (int32_t i = 0 ; i < 42 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    gfmul_42(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 42 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    W0[43] = U2[41];
    W4[43] = V2[41];
    for (int32_t i = 0 ; i < 42 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[42] = W0[42];
    W3[43] = W0[43];
    W2[42] = W4[42];
    W2[43] = W4[43];
    for (int32_t i = 0 ; i < 42 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_44(tmp, W3, W2);
    for (int32_t i = 0 ; i < 88 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_44(W2, W0, W4);
    gfmul_42(W4, U2, V2);
    gfmul_42(W0, U0, V0);
    for (int32_t i = 0 ; i < 88 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 84 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 83 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[83] = W2[84];
    W2[84] = W2[85];
    W2[85] = W2[86];
    W2[86] = W2[87];
    for (int32_t i = 0 ; i < 84 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 84 ; i < 87 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[87] = W3[87];
    for (int32_t i = 0 ; i < 84 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 88);
    for (int32_t i = 0 ; i < 83 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[83] = W3[84];
    tmp[84] = W3[85];
    tmp[85] = W3[86];
    tmp[86] = W3[87];
    divide_by_x_plus_one(tmp, W3, 87);
    for (int32_t i = 0 ; i < 84 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 84 ; i < 87 ; i++) {
        W1[i] = W2[i];
    }
    for (int32_t i = 0 ; i < 86 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 42; i++) {
        int32_t j = i + 42;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 42] = W1[j] ^ W2[i];
        Out[j + 84] = W2[j] ^ W3[i];
        Out[i + 168] = W3[j] ^ W4[i];
        Out[j + 168] = W4[j];
    }
    Out[126] ^= W1[84];
    Out[127] ^= W1[85];
    Out[128] ^= W1[86];
    Out[168] ^= W2[84];
    Out[169] ^= W2[85];
    Out[170] ^= W2[86];
    Out[210] ^= W3[84];
    Out[211] ^= W3[85];
}

#define gfmul_127 tc3_127
static inline void tc3_127(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[86], W1[87], W2[88], W3[88], W4[82], tmp[88];
    U0 = &A[0];
    U1 = &A[43];
    U2 = &A[86];
    V0 = &B[0];
    V1 = &B[43];
    V2 = &B[86];
    for (int32_t i = 0 ; i < 41 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[41] = U0[41] ^ U1[41];
    W2[41] = V0[41] ^ V1[41];
    W3[42] = U0[42] ^ U1[42];
    W2[42] = V0[42] ^ V1[42];
    gfmul_43(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 42 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    
    W0[43] = U1[42];
    W4[43] = V1[42];
    for (int32_t i = 0 ; i < 43 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[43] = W0[43];
    W2[43] = W4[43];
    for (int32_t i = 0 ; i < 43 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_44(tmp, W3, W2);
    for (int32_t i = 0 ; i < 88 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_44(W2, W0, W4);
    gfmul_41(W4, U2, V2);
    gfmul_43(W0, U0, V0);
    for (int32_t i = 0 ; i < 88 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 86 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 85 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[85] = W2[86];
    W2[86] = W2[87];
    for (int32_t i = 0 ; i < 82 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 82 ; i < 87 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[87] = W3[87];
    for (int32_t i = 0 ; i < 82 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 88);
    for (int32_t i = 0 ; i < 85 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[85] = W3[86];
    tmp[86] = W3[87];
    divide_by_x_plus_one(tmp, W3, 87);
    for (int32_t i = 0 ; i < 82 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 82 ; i < 86 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[86] = W2[86];
    for (int32_t i = 0 ; i < 86 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 39; i++) {
        int32_t j = i + 43;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 43] = W1[j] ^ W2[i];
        Out[j + 86] = W2[j] ^ W3[i];
        Out[i + 172] = W3[j] ^ W4[i];
        Out[j + 172] = W4[j];
    }
    for (int32_t i = 39; i < 43; i++) {
        int32_t j = i + 43;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 43] = W1[j] ^ W2[i];
        Out[j + 86] = W2[j] ^ W3[i];
        Out[i + 172] = W3[j] ^ W4[i];
    }
    Out[129] ^= W1[86];
    Out[172] ^= W2[86];
}

#define gfmul_373 tc3_373
static inline void tc3_373(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[250], W1[251], W2[252], W3[252], W4[246], tmp[252];
    U0 = &A[0];
    U1 = &A[125];
    U2 = &A[250];
    V0 = &B[0];
    V1 = &B[125];
    V2 = &B[250];
    for (int32_t i = 0 ; i < 123 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[123] = U0[123] ^ U1[123];
    W2[123] = V0[123] ^ V1[123];
    W3[124] = U0[124] ^ U1[124];
    W2[124] = V0[124] ^ V1[124];
    gfmul_125(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 124 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    
    W0[125] = U1[124];
    W4[125] = V1[124];
    for (int32_t i = 0 ; i < 125 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[125] = W0[125];
    W2[125] = W4[125];
    for (int32_t i = 0 ; i < 125 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_126(tmp, W3, W2);
    for (int32_t i = 0 ; i < 252 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_126(W2, W0, W4);
    gfmul_123(W4, U2, V2);
    gfmul_125(W0, U0, V0);
    for (int32_t i = 0 ; i < 252 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 250 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 249 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[249] = W2[250];
    W2[250] = W2[251];
    for (int32_t i = 0 ; i < 246 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 246 ; i < 251 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[251] = W3[251];
    for (int32_t i = 0 ; i < 246 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 252);
    for (int32_t i = 0 ; i < 249 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[249] = W3[250];
    tmp[250] = W3[251];
    divide_by_x_plus_one(tmp, W3, 251);
    for (int32_t i = 0 ; i < 246 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 246 ; i < 250 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[250] = W2[250];
    for (int32_t i = 0 ; i < 250 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 121; i++) {
        int32_t j = i + 125;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 125] = W1[j] ^ W2[i];
        Out[j + 250] = W2[j] ^ W3[i];
        Out[i + 500] = W3[j] ^ W4[i];
        Out[j + 500] = W4[j];
    }
    for (int32_t i = 121; i < 125; i++) {
        int32_t j = i + 125;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 125] = W1[j] ^ W2[i];
        Out[j + 250] = W2[j] ^ W3[i];
        Out[i + 500] = W3[j] ^ W4[i];
    }
    Out[375] ^= W1[250];
    Out[500] ^= W2[250];
}

#define gfmul_374 tc3_374
static inline void tc3_374(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
        static uint32_t W0[250], W1[251], W2[252], W3[252], W4[248], tmp[252];
    U0 = &A[0];
    U1 = &A[125];
    U2 = &A[250];
    V0 = &B[0];
    V1 = &B[125];
    V2 = &B[250];
    for (int32_t i = 0 ; i < 124 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[124] = U0[124] ^ U1[124];
    W2[124] = V0[124] ^ V1[124];
    gfmul_125(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 125 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    for (int32_t i = 0 ; i < 125 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[125] = W0[125];
    W2[125] = W4[125];
    for (int32_t i = 0 ; i < 125 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_126(tmp, W3, W2);
    for (int32_t i = 0 ; i < 252 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_126(W2, W0, W4);
    gfmul_124(W4, U2, V2);
    gfmul_125(W0, U0, V0);
    for (int32_t i = 0 ; i < 252 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 250 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 249 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[249] = W2[250];
    W2[250] = W2[251];
    for (int32_t i = 0 ; i < 248 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 248 ; i < 251 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[251] = W3[251];
    for (int32_t i = 0 ; i < 248 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 252);
    for (int32_t i = 0 ; i < 249 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[249] = W3[250];
    tmp[250] = W3[251];
    divide_by_x_plus_one(tmp, W3, 251);
    for (int32_t i = 0 ; i < 248 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 248 ; i < 250 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[250] = W2[250];
    for (int32_t i = 0 ; i < 250 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 123; i++) {
        int32_t j = i + 125;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 125] = W1[j] ^ W2[i];
        Out[j + 250] = W2[j] ^ W3[i];
        Out[i + 500] = W3[j] ^ W4[i];
        Out[j + 500] = W4[j];
    }
    for (int32_t i = 123; i < 125; i++) {
        int32_t j = i + 125;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 125] = W1[j] ^ W2[i];
        Out[j + 250] = W2[j] ^ W3[i];
        Out[i + 500] = W3[j] ^ W4[i];
    }
    Out[375] ^= W1[250];
    Out[500] ^= W2[250];
}

#define gfmul_375 tc3_375
static inline void tc3_375(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[250], W1[253], W2[254], W3[254], W4[250], tmp[254];
    U0 = &A[0];
    U1 = &A[125];
    U2 = &A[250];
    V0 = &B[0];
    V1 = &B[125];
    V2 = &B[250];
    for (int32_t i = 0 ; i < 125 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    gfmul_125(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 125 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    W0[126] = U2[124];
    W4[126] = V2[124];
    for (int32_t i = 0 ; i < 125 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[125] = W0[125];
    W3[126] = W0[126];
    W2[125] = W4[125];
    W2[126] = W4[126];
    for (int32_t i = 0 ; i < 125 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_127(tmp, W3, W2);
    for (int32_t i = 0 ; i < 254 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_127(W2, W0, W4);
    gfmul_125(W4, U2, V2);
    gfmul_125(W0, U0, V0);
    for (int32_t i = 0 ; i < 254 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 250 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 249 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[249] = W2[250];
    W2[250] = W2[251];
    W2[251] = W2[252];
    W2[252] = W2[253];
    for (int32_t i = 0 ; i < 250 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 250 ; i < 253 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[253] = W3[253];
    for (int32_t i = 0 ; i < 250 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 254);
    for (int32_t i = 0 ; i < 249 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[249] = W3[250];
    tmp[250] = W3[251];
    tmp[251] = W3[252];
    tmp[252] = W3[253];
    divide_by_x_plus_one(tmp, W3, 253);
    for (int32_t i = 0 ; i < 250 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 250 ; i < 253 ; i++) {
        W1[i] = W2[i];
    }
    for (int32_t i = 0 ; i < 252 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 125; i++) {
        int32_t j = i + 125;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 125] = W1[j] ^ W2[i];
        Out[j + 250] = W2[j] ^ W3[i];
        Out[i + 500] = W3[j] ^ W4[i];
        Out[j + 500] = W4[j];
    }
    Out[375] ^= W1[250];
    Out[376] ^= W1[251];
    Out[377] ^= W1[252];
    Out[500] ^= W2[250];
    Out[501] ^= W2[251];
    Out[502] ^= W2[252];
    Out[625] ^= W3[250];
    Out[626] ^= W3[251];
}

#define gfmul_1121 tc3_1121
static inline void tc3_1121(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
        static uint32_t W0[748], W1[749], W2[750], W3[750], W4[746], tmp[750];
    U0 = &A[0];
    U1 = &A[374];
    U2 = &A[748];
    V0 = &B[0];
    V1 = &B[374];
    V2 = &B[748];
    for (int32_t i = 0 ; i < 373 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[373] = U0[373] ^ U1[373];
    W2[373] = V0[373] ^ V1[373];
    gfmul_374(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 374 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    for (int32_t i = 0 ; i < 374 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[374] = W0[374];
    W2[374] = W4[374];
    for (int32_t i = 0 ; i < 374 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_375(tmp, W3, W2);
    for (int32_t i = 0 ; i < 750 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_375(W2, W0, W4);
    gfmul_373(W4, U2, V2);
    gfmul_374(W0, U0, V0);
    for (int32_t i = 0 ; i < 750 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 748 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 747 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[747] = W2[748];
    W2[748] = W2[749];
    for (int32_t i = 0 ; i < 746 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 746 ; i < 749 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[749] = W3[749];
    for (int32_t i = 0 ; i < 746 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 750);
    for (int32_t i = 0 ; i < 747 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[747] = W3[748];
    tmp[748] = W3[749];
    divide_by_x_plus_one(tmp, W3, 749);
    for (int32_t i = 0 ; i < 746 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 746 ; i < 748 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[748] = W2[748];
    for (int32_t i = 0 ; i < 748 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 372; i++) {
        int32_t j = i + 374;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 374] = W1[j] ^ W2[i];
        Out[j + 748] = W2[j] ^ W3[i];
        Out[i + 1496] = W3[j] ^ W4[i];
        Out[j + 1496] = W4[j];
    }
    for (int32_t i = 372; i < 374; i++) {
        int32_t j = i + 374;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 374] = W1[j] ^ W2[i];
        Out[j + 748] = W2[j] ^ W3[i];
        Out[i + 1496] = W3[j] ^ W4[i];
    }
    Out[1122] ^= W1[748];
    Out[1496] ^= W2[748];
}


/**
 * @brief Multiply two polynomials modulo \f$ X^n - 1\f$.
 *
 * This functions multiplies polynomials <b>v1</b> and <b>v2</b>.
 * The multiplication is done modulo \f$ X^n - 1\f$.
 *
 * @param[out] o Product of <b>v1</b> and <b>v2</b>
 * @param[in] v1 Pointer to the first polynomial
 * @param[in] v2 Pointer to the second polynomial
 */
void PQCLEAN_HQC192_CLEAN_vect_mul(uint64_t *o, const uint64_t *v1, const uint64_t *v2) {
	
	uint32_t stk[1121<<1];
	uint32_t *sa = stk, *sb = stk + 1121;
    
	poly_swap(sa, (uint32_t*)v1, 1121);
	poly_swap(sb, (uint32_t*)v2, 1121);
	gfmul_1121(stk, sa, sb);
	poly_invswap(stk, 2*1121);

	reduce((uint32_t*)o, (uint32_t*)stk);    
}
