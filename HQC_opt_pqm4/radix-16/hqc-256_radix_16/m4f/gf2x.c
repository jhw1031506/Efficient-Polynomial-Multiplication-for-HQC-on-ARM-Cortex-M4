#include "gf2x.h"
#include "parameters.h"
#include <stddef.h>
#include <stdint.h>
#include "reduce.h"
#include <string.h>

//CASE can be 1,2,3,4,5
#ifndef CASE
    #define CASE 2
#endif

#if CASE==1 
#define WMUL 1
#elif CASE==2 //default
#define WMUL 2
#elif CASE==3
#define WMUL 3
#elif CASE==4
#define WMUL 4
#elif CASE==5
#define WMUL 5
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
	for(int i=0;i<len;i++){
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
	for(int i=0;i<len;i++){
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

#if WMUL !=3
#define gfmul_9 karat3_9
static inline void karat3_9(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
  const uint32_t *a0, *a1, *a2, *b0, *b1, *b2;
  static uint32_t aa01[3], bb01[3], aa02[3], bb02[3], aa12[3], bb12[3], middle;
  static uint32_t D0[6], D1[6], D2[6], D3[6], D4[6], D5[6];
  a0 = A;
  a1 = A + 3;
  a2 = A + 6;
  b0 = B;
  b1 = B + 3;
  b2 = B + 6;
  for (int16_t i = 0; i < 3; i++)
  {
      aa01[i] = a0[i] ^ a1[i];
      bb01[i] = b0[i] ^ b1[i];
      aa12[i] = a2[i] ^ a1[i];
      bb12[i] = b2[i] ^ b1[i];
      aa02[i] = a0[i] ^ a2[i];
      bb02[i] = b0[i] ^ b2[i];
  }
  gfmul_3(D3, aa01, bb01);
  gfmul_3(D4, aa02, bb02);
  gfmul_3(D5, aa12, bb12);
  gfmul_3(D0, a0, b0);
  gfmul_3(D1, a1, b1);
  gfmul_3(D2, a2, b2);
  for (int16_t i = 0; i < 3; i++)
  {
      int16_t j = i + 3;
      middle = D0[i] ^ D1[i] ^ D0[j];
      Out[i] = D0[i];
      Out[j] = D3[i] ^ middle;
      Out[j + 3] = D4[i] ^ D2[i] ^ D3[j] ^ D1[j] ^ middle;
      middle = D1[j] ^ D2[i] ^ D2[j];
      Out[j + 6] = D5[i] ^ D4[j] ^ D0[j] ^ D1[i] ^ middle;
      Out[i + 12] = D5[j] ^ middle;
      Out[j + 12] = D2[j];
  }
}
#endif

#if WMUL!=5
#define gfmul_11 karat2_11
static inline void karat2_11(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[12], D1[12], D2[10], SAA[6], SBB[6];
    gfmul_6(D0, A, B);
    gfmul_5(D2, (A+6), (B+6));
    for(int32_t i = 0; i < 5; i++) {
        int32_t is = i + 6;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
        SAA[5]=A[5];
        SBB[5]=B[5];
    gfmul_6(D1, SAA, SBB);
    for(int32_t i = 0; i < 4; i++) {
        int32_t is = i + 6;
        int32_t is2 = is + 6;
        int32_t is3 = is2 + 6;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
    for(int32_t i = 4; i < 6; i++) {
        int32_t is = i + 6;
        int32_t is2 = is + 6;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is];
    }
}
#endif

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
#if WMUL == 2 || WMUL == 1

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
#endif
#if WMUL == 1 || WMUL == 2
#define gfmul_17 karat2_17
static inline void karat2_17(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[18], D1[18], D2[16], SAA[9], SBB[9];
    gfmul_9(D0, A, B);
    gfmul_8(D2, (A+9), (B+9));
    for(int32_t i = 0; i < 8; i++) {
        int32_t is = i + 9;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
        SAA[8]=A[8];
        SBB[8]=B[8];
    gfmul_9(D1, SAA, SBB);
    for(int32_t i = 0; i < 7; i++) {
        int32_t is = i + 9;
        int32_t is2 = is + 9;
        int32_t is3 = is2 + 9;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
    for(int32_t i = 7; i < 9; i++) {
        int32_t is = i + 9;
        int32_t is2 = is + 9;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is];
    }
}
#endif
#define gfmul_21 karat3_21
static inline void karat3_21(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
  const uint32_t *a0, *a1, *a2, *b0, *b1, *b2;
  static uint32_t aa01[7], bb01[7], aa02[7], bb02[7], aa12[7], bb12[7], middle;
  static uint32_t D0[14], D1[14], D2[14], D3[14], D4[14], D5[14];
  a0 = A;
  a1 = A + 7;
  a2 = A + 14;
  b0 = B;
  b1 = B + 7;
  b2 = B + 14;
  for (int16_t i = 0; i < 7; i++)
  {
      aa01[i] = a0[i] ^ a1[i];
      bb01[i] = b0[i] ^ b1[i];
      aa12[i] = a2[i] ^ a1[i];
      bb12[i] = b2[i] ^ b1[i];
      aa02[i] = a0[i] ^ a2[i];
      bb02[i] = b0[i] ^ b2[i];
  }
  gfmul_7(D3, aa01, bb01);
  gfmul_7(D4, aa02, bb02);
  gfmul_7(D5, aa12, bb12);
  gfmul_7(D0, a0, b0);
  gfmul_7(D1, a1, b1);
  gfmul_7(D2, a2, b2);
  for (int16_t i = 0; i < 7; i++)
  {
      int16_t j = i + 7;
      middle = D0[i] ^ D1[i] ^ D0[j];
      Out[i] = D0[i];
      Out[j] = D3[i] ^ middle;
      Out[j + 7] = D4[i] ^ D2[i] ^ D3[j] ^ D1[j] ^ middle;
      middle = D1[j] ^ D2[i] ^ D2[j];
      Out[j + 14] = D5[i] ^ D4[j] ^ D0[j] ^ D1[i] ^ middle;
      Out[i + 28] = D5[j] ^ middle;
      Out[j + 28] = D2[j];
  }
}
#if WMUL == 5
#define gfmul_22 tc3_22
static inline void tc3_22(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
  const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
  static uint32_t W0[16], W1[17], W2[18], W3[18], W4[12], tmp[18];
  U0 = &A[0];
  U1 = &A[8];
  U2 = &A[16];
  V0 = &B[0];
  V1 = &B[8];
  V2 = &B[16];
  for (int32_t i = 0 ; i < 6 ; i++) {
      W3[i] = U0[i] ^ U1[i] ^ U2[i];
      W2[i] = V0[i] ^ V1[i] ^ V2[i];
  }
  W3[6] = U0[6] ^ U1[6];
  W2[6] = V0[6] ^ V1[6];
  W3[7] = U0[7] ^ U1[7];
  W2[7] = V0[7] ^ V1[7];
  gfmul_8(W1, W2, W3);
  W0[0] = 0;
  W4[0] = 0;
  W0[1] = U1[0];
  W4[1] = V1[0];
  for (int32_t i = 1 ; i < 7 ; i++) {
      W0[i + 1] = U1[i] ^ U2[i - 1];
      W4[i + 1] = V1[i] ^ V2[i - 1];
  }

  W0[8] = U1[7];
  W4[8] = V1[7];
  for (int32_t i = 0 ; i < 8 ; i++) {
      W3[i] ^= W0[i];
      W2[i] ^= W4[i];
  }
  W3[8] = W0[8];
  W2[8] = W4[8];
  for (int32_t i = 0 ; i < 8 ; i++) {
      W0[i] ^= U0[i];
      W4[i] ^= V0[i];
  }
  gfmul_9(tmp, W3, W2);
  for (int32_t i = 0 ; i < 18 ; i++) {
      W3[i] = tmp[i];
  }
  gfmul_9(W2, W0, W4);
  gfmul_6(W4, U2, V2);
  gfmul_8(W0, U0, V0);
  for (int32_t i = 0 ; i < 18 ; i++) {
      W3[i] ^= W2[i];
  }
  for (int32_t i = 0 ; i < 16 ; i++) {
      W1[i] ^= W0[i];
  }
  for (int32_t i = 0 ; i < 15 ; i++) {
      int32_t i1 = i + 1;
      W2[i] = W2[i1] ^ W0[i1];
  }
  W2[15] = W2[16];
  W2[16] = W2[17];
  for (int32_t i = 0 ; i < 12 ; i++) {
      tmp[i] = W2[i] ^ W3[i] ^ W4[i];
  }
  for (int32_t i = 12 ; i < 17 ; i++) {
      tmp[i] = W2[i] ^ W3[i];
  }
  tmp[17] = W3[17];
  for (int32_t i = 0 ; i < 12 ; i++) {
      tmp[i + 3] ^= W4[i];
  }
  divide_by_x_plus_one(tmp, W2, 18);
  for (int32_t i = 0 ; i < 15 ; i++) {
      int32_t i1 = i + 1;
      tmp[i] = W3[i1] ^ W1[i1];
  }
  tmp[15] = W3[16];
  tmp[16] = W3[17];
  divide_by_x_plus_one(tmp, W3, 17);
  for (int32_t i = 0 ; i < 12 ; i++) {
      W1[i] ^= W2[i] ^ W4[i];
  }
  for (int32_t i = 12 ; i < 16 ; i++) {
      W1[i] ^= W2[i];
  }
  W1[16] = W2[16];
  for (int32_t i = 0 ; i < 16 ; i++) {
      W2[i] ^= W3[i];
  }
  for (int32_t i = 0; i < 4; i++) {
      int32_t j = i + 8;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 8] = W1[j] ^ W2[i];
      Out[j + 16] = W2[j] ^ W3[i];
      Out[i + 32] = W3[j] ^ W4[i];
      Out[j + 32] = W4[j];
  }
  for (int32_t i = 4; i < 8; i++) {
      int32_t j = i + 8;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 8] = W1[j] ^ W2[i];
      Out[j + 16] = W2[j] ^ W3[i];
      Out[i + 32] = W3[j] ^ W4[i];
  }
  Out[24] ^= W1[16];
  Out[32] ^= W2[16];
}
#else
#define gfmul_22 karat2_22
static inline void karat2_22(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[22], D1[22], D2[22], SAA[11], SBB[11];
    gfmul_11(D0, A, B);
    gfmul_11(D2, (A+11), (B+11));
    for(int32_t i = 0; i < 11; i++) {
        int32_t is = i + 11;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
    gfmul_11(D1, SAA, SBB);
    for(int32_t i = 0; i < 11; i++) {
        int32_t is = i + 11;
        int32_t is2 = is + 11;
        int32_t is3 = is2 + 11;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
}
#endif

#if WMUL == 5 || WMUL == 4
#define gfmul_23 tc3_23
static inline void tc3_23(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
  const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
      static uint32_t W0[16], W1[17], W2[18], W3[18], W4[14], tmp[18];
  U0 = &A[0];
  U1 = &A[8];
  U2 = &A[16];
  V0 = &B[0];
  V1 = &B[8];
  V2 = &B[16];
  for (int32_t i = 0 ; i < 7 ; i++) {
      W3[i] = U0[i] ^ U1[i] ^ U2[i];
      W2[i] = V0[i] ^ V1[i] ^ V2[i];
  }
  W3[7] = U0[7] ^ U1[7];
  W2[7] = V0[7] ^ V1[7];
  gfmul_8(W1, W2, W3);
  W0[0] = 0;
  W4[0] = 0;
  W0[1] = U1[0];
  W4[1] = V1[0];
  for (int32_t i = 1 ; i < 8 ; i++) {
      W0[i + 1] = U1[i] ^ U2[i - 1];
      W4[i + 1] = V1[i] ^ V2[i - 1];
  }
  for (int32_t i = 0 ; i < 8 ; i++) {
      W3[i] ^= W0[i];
      W2[i] ^= W4[i];
  }
  W3[8] = W0[8];
  W2[8] = W4[8];
  for (int32_t i = 0 ; i < 8 ; i++) {
      W0[i] ^= U0[i];
      W4[i] ^= V0[i];
  }
  gfmul_9(tmp, W3, W2);
  for (int32_t i = 0 ; i < 18 ; i++) {
      W3[i] = tmp[i];
  }
  gfmul_9(W2, W0, W4);
  gfmul_7(W4, U2, V2);
  gfmul_8(W0, U0, V0);
  for (int32_t i = 0 ; i < 18 ; i++) {
      W3[i] ^= W2[i];
  }
  for (int32_t i = 0 ; i < 16 ; i++) {
      W1[i] ^= W0[i];
  }
  for (int32_t i = 0 ; i < 15 ; i++) {
      int32_t i1 = i + 1;
      W2[i] = W2[i1] ^ W0[i1];
  }
  W2[15] = W2[16];
  W2[16] = W2[17];
  for (int32_t i = 0 ; i < 14 ; i++) {
      tmp[i] = W2[i] ^ W3[i] ^ W4[i];
  }
  for (int32_t i = 14 ; i < 17 ; i++) {
      tmp[i] = W2[i] ^ W3[i];
  }
  tmp[17] = W3[17];
  for (int32_t i = 0 ; i < 14 ; i++) {
      tmp[i + 3] ^= W4[i];
  }
  divide_by_x_plus_one(tmp, W2, 18);
  for (int32_t i = 0 ; i < 15 ; i++) {
      int32_t i1 = i + 1;
      tmp[i] = W3[i1] ^ W1[i1];
  }
  tmp[15] = W3[16];
  tmp[16] = W3[17];
  divide_by_x_plus_one(tmp, W3, 17);
  for (int32_t i = 0 ; i < 14 ; i++) {
      W1[i] ^= W2[i] ^ W4[i];
  }
  for (int32_t i = 14 ; i < 16 ; i++) {
      W1[i] ^= W2[i];
  }
  W1[16] = W2[16];
  for (int32_t i = 0 ; i < 16 ; i++) {
      W2[i] ^= W3[i];
  }
  for (int32_t i = 0; i < 6; i++) {
      int32_t j = i + 8;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 8] = W1[j] ^ W2[i];
      Out[j + 16] = W2[j] ^ W3[i];
      Out[i + 32] = W3[j] ^ W4[i];
      Out[j + 32] = W4[j];
  }
  for (int32_t i = 6; i < 8; i++) {
      int32_t j = i + 8;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 8] = W1[j] ^ W2[i];
      Out[j + 16] = W2[j] ^ W3[i];
      Out[i + 32] = W3[j] ^ W4[i];
  }
  Out[24] ^= W1[16];
  Out[32] ^= W2[16];
}
#else
#define gfmul_23 karat2_23
static inline void karat2_23(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[24], D1[24], D2[22], SAA[12], SBB[12];
    gfmul_12(D0, A, B);
    gfmul_11(D2, (A+12), (B+12));
    for(int32_t i = 0; i < 11; i++) {
        int32_t is = i + 12;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
        SAA[11]=A[11];
        SBB[11]=B[11];
    gfmul_12(D1, SAA, SBB);
    for(int32_t i = 0; i < 10; i++) {
        int32_t is = i + 12;
        int32_t is2 = is + 12;
        int32_t is3 = is2 + 12;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
    for(int32_t i = 10; i < 12; i++) {
        int32_t is = i + 12;
        int32_t is2 = is + 12;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is];
    }
}
#endif
#define gfmul_24 karat2_24
static inline void karat2_24(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
  static uint32_t D0[24], D1[24], D2[24], SAA[12], SBB[12];
  gfmul_12(D0, A, B);
  gfmul_12(D2, (A+12), (B+12));
  for(int32_t i = 0; i < 12; i++) {
      int32_t is = i + 12;
      SAA[i] = A[i] ^ A[is];
      SBB[i] = B[i] ^ B[is];
  }
  gfmul_12(D1, SAA, SBB);
  for(int32_t i = 0; i < 12; i++) {
      int32_t is = i + 12;
      int32_t is2 = is + 12;
      int32_t is3 = is2 + 12;
      uint32_t middle = D0[is] ^ D2[i];
      Out[i]   = D0[i];
      Out[is]  = middle ^ D0[i] ^ D1[i];
      Out[is2] = middle ^ D1[is] ^ D2[is];
      Out[is3] = D2[is];
  }
}

#define gfmul_25 karat2_25
static inline void karat2_25(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
  static uint32_t D0[26], D1[26], D2[24], SAA[13], SBB[13];
  gfmul_13(D0, A, B);
  gfmul_12(D2, (A+13), (B+13));
  for(int32_t i = 0; i < 12; i++) {
      int32_t is = i + 13;
      SAA[i] = A[i] ^ A[is];
      SBB[i] = B[i] ^ B[is];
  }
      SAA[12]=A[12];
      SBB[12]=B[12];
  gfmul_13(D1, SAA, SBB);
  for(int32_t i = 0; i < 11; i++) {
      int32_t is = i + 13;
      int32_t is2 = is + 13;
      int32_t is3 = is2 + 13;
      uint32_t middle = D0[is] ^ D2[i];
      Out[i]   = D0[i];
      Out[is]  = middle ^ D0[i] ^ D1[i];
      Out[is2] = middle ^ D1[is] ^ D2[is];
      Out[is3] = D2[is];
  }
  for(int32_t i = 11; i < 13; i++) {
      int32_t is = i + 13;
      int32_t is2 = is + 13;
      uint32_t middle = D0[is] ^ D2[i];
      Out[i]   = D0[i];
      Out[is]  = middle ^ D0[i] ^ D1[i];
      Out[is2] = middle ^ D1[is];
  }
}


#if WMUL == 1 || WMUL == 2

#define gfmul_32 karat2_32
static inline void karat2_32(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
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

#if WMUL == 1 || WMUL == 2
#define gfmul_33 karat2_33
static inline void karat2_33(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[34], D1[34], D2[32], SAA[17], SBB[17];
    gfmul_17(D0, A, B);
    gfmul_16(D2, (A+17), (B+17));
    for(int32_t i = 0; i < 16; i++) {
        int32_t is = i + 17;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
        SAA[16]=A[16];
        SBB[16]=B[16];
    gfmul_17(D1, SAA, SBB);
    for(int32_t i = 0; i < 15; i++) {
        int32_t is = i + 17;
        int32_t is2 = is + 17;
        int32_t is3 = is2 + 17;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
    for(int32_t i = 15; i < 17; i++) {
        int32_t is = i + 17;
        int32_t is2 = is + 17;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is];
    }
}

#endif

#if WMUL == 5 || WMUL == 4 || WMUL == 3
#define gfmul_65 tc3_65
static inline void tc3_65(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
  const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
      static uint32_t W0[44], W1[45], W2[46], W3[46], W4[42], tmp[46];
  U0 = &A[0];
  U1 = &A[22];
  U2 = &A[44];
  V0 = &B[0];
  V1 = &B[22];
  V2 = &B[44];
  for (int32_t i = 0 ; i < 21 ; i++) {
      W3[i] = U0[i] ^ U1[i] ^ U2[i];
      W2[i] = V0[i] ^ V1[i] ^ V2[i];
  }
  W3[21] = U0[21] ^ U1[21];
  W2[21] = V0[21] ^ V1[21];
  gfmul_22(W1, W2, W3);
  W0[0] = 0;
  W4[0] = 0;
  W0[1] = U1[0];
  W4[1] = V1[0];
  for (int32_t i = 1 ; i < 22 ; i++) {
      W0[i + 1] = U1[i] ^ U2[i - 1];
      W4[i + 1] = V1[i] ^ V2[i - 1];
  }
  for (int32_t i = 0 ; i < 22 ; i++) {
      W3[i] ^= W0[i];
      W2[i] ^= W4[i];
  }
  W3[22] = W0[22];
  W2[22] = W4[22];
  for (int32_t i = 0 ; i < 22 ; i++) {
      W0[i] ^= U0[i];
      W4[i] ^= V0[i];
  }
  gfmul_23(tmp, W3, W2);
  for (int32_t i = 0 ; i < 46 ; i++) {
      W3[i] = tmp[i];
  }
  gfmul_23(W2, W0, W4);
  gfmul_21(W4, U2, V2);
  gfmul_22(W0, U0, V0);
  for (int32_t i = 0 ; i < 46 ; i++) {
      W3[i] ^= W2[i];
  }
  for (int32_t i = 0 ; i < 44 ; i++) {
      W1[i] ^= W0[i];
  }
  for (int32_t i = 0 ; i < 43 ; i++) {
      int32_t i1 = i + 1;
      W2[i] = W2[i1] ^ W0[i1];
  }
  W2[43] = W2[44];
  W2[44] = W2[45];
  for (int32_t i = 0 ; i < 42 ; i++) {
      tmp[i] = W2[i] ^ W3[i] ^ W4[i];
  }
  for (int32_t i = 42 ; i < 45 ; i++) {
      tmp[i] = W2[i] ^ W3[i];
  }
  tmp[45] = W3[45];
  for (int32_t i = 0 ; i < 42 ; i++) {
      tmp[i + 3] ^= W4[i];
  }
  divide_by_x_plus_one(tmp, W2, 46);
  for (int32_t i = 0 ; i < 43 ; i++) {
      int32_t i1 = i + 1;
      tmp[i] = W3[i1] ^ W1[i1];
  }
  tmp[43] = W3[44];
  tmp[44] = W3[45];
  divide_by_x_plus_one(tmp, W3, 45);
  for (int32_t i = 0 ; i < 42 ; i++) {
      W1[i] ^= W2[i] ^ W4[i];
  }
  for (int32_t i = 42 ; i < 44 ; i++) {
      W1[i] ^= W2[i];
  }
  W1[44] = W2[44];
  for (int32_t i = 0 ; i < 44 ; i++) {
      W2[i] ^= W3[i];
  }
  for (int32_t i = 0; i < 20; i++) {
      int32_t j = i + 22;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 22] = W1[j] ^ W2[i];
      Out[j + 44] = W2[j] ^ W3[i];
      Out[i + 88] = W3[j] ^ W4[i];
      Out[j + 88] = W4[j];
  }
  for (int32_t i = 20; i < 22; i++) {
      int32_t j = i + 22;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 22] = W1[j] ^ W2[i];
      Out[j + 44] = W2[j] ^ W3[i];
      Out[i + 88] = W3[j] ^ W4[i];
  }
  Out[66] ^= W1[44];
  Out[88] ^= W2[44];
}
#else
#define gfmul_65 karat2_65
static inline void karat2_65(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[66], D1[66], D2[64], SAA[33], SBB[33];
    gfmul_33(D0, A, B);
    gfmul_32(D2, (A+33), (B+33));
    for(int32_t i = 0; i < 32; i++) {
        int32_t is = i + 33;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
        SAA[32]=A[32];
        SBB[32]=B[32];
    gfmul_33(D1, SAA, SBB);
    for(int32_t i = 0; i < 31; i++) {
        int32_t is = i + 33;
        int32_t is2 = is + 33;
        int32_t is3 = is2 + 33;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
    for(int32_t i = 31; i < 33; i++) {
        int32_t is = i + 33;
        int32_t is2 = is + 33;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is];
    }
}
#endif

#if WMUL != 1
#define gfmul_66 tc3_66
static inline void tc3_66(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
  const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
  static uint32_t W0[44], W1[47], W2[48], W3[48], W4[44], tmp[48];
  U0 = &A[0];
  U1 = &A[22];
  U2 = &A[44];
  V0 = &B[0];
  V1 = &B[22];
  V2 = &B[44];
  for (int32_t i = 0 ; i < 22 ; i++) {
      W3[i] = U0[i] ^ U1[i] ^ U2[i];
      W2[i] = V0[i] ^ V1[i] ^ V2[i];
  }
  gfmul_22(W1, W2, W3);
  W0[0] = 0;
  W4[0] = 0;
  W0[1] = U1[0];
  W4[1] = V1[0];
  for (int32_t i = 1 ; i < 22 ; i++) {
      W0[i + 1] = U1[i] ^ U2[i - 1];
      W4[i + 1] = V1[i] ^ V2[i - 1];
  }
  W0[23] = U2[21];
  W4[23] = V2[21];
  for (int32_t i = 0 ; i < 22 ; i++) {
      W3[i] ^= W0[i];
      W2[i] ^= W4[i];
  }
  W3[22] = W0[22];
  W3[23] = W0[23];
  W2[22] = W4[22];
  W2[23] = W4[23];
  for (int32_t i = 0 ; i < 22 ; i++) {
      W0[i] ^= U0[i];
      W4[i] ^= V0[i];
  }
  gfmul_24(tmp, W3, W2);
  for (int32_t i = 0 ; i < 48 ; i++) {
      W3[i] = tmp[i];
  }
  gfmul_24(W2, W0, W4);
  gfmul_22(W4, U2, V2);
  gfmul_22(W0, U0, V0);
  for (int32_t i = 0 ; i < 48 ; i++) {
      W3[i] ^= W2[i];
  }
  for (int32_t i = 0 ; i < 44 ; i++) {
      W1[i] ^= W0[i];
  }
  for (int32_t i = 0 ; i < 43 ; i++) {
      int32_t i1 = i + 1;
      W2[i] = W2[i1] ^ W0[i1];
  }
  W2[43] = W2[44];
  W2[44] = W2[45];
  W2[45] = W2[46];
  W2[46] = W2[47];
  for (int32_t i = 0 ; i < 44 ; i++) {
      tmp[i] = W2[i] ^ W3[i] ^ W4[i];
  }
  for (int32_t i = 44 ; i < 47 ; i++) {
      tmp[i] = W2[i] ^ W3[i];
  }
  tmp[47] = W3[47];
  for (int32_t i = 0 ; i < 44 ; i++) {
      tmp[i + 3] ^= W4[i];
  }
  divide_by_x_plus_one(tmp, W2, 48);
  for (int32_t i = 0 ; i < 43 ; i++) {
      int32_t i1 = i + 1;
      tmp[i] = W3[i1] ^ W1[i1];
  }
  tmp[43] = W3[44];
  tmp[44] = W3[45];
  tmp[45] = W3[46];
  tmp[46] = W3[47];
  divide_by_x_plus_one(tmp, W3, 47);
  for (int32_t i = 0 ; i < 44 ; i++) {
      W1[i] ^= W2[i] ^ W4[i];
  }
  for (int32_t i = 44 ; i < 47 ; i++) {
      W1[i] = W2[i];
  }
  for (int32_t i = 0 ; i < 46 ; i++) {
      W2[i] ^= W3[i];
  }
  for (int32_t i = 0; i < 22; i++) {
      int32_t j = i + 22;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 22] = W1[j] ^ W2[i];
      Out[j + 44] = W2[j] ^ W3[i];
      Out[i + 88] = W3[j] ^ W4[i];
      Out[j + 88] = W4[j];
  }
  Out[66] ^= W1[44];
  Out[67] ^= W1[45];
  Out[68] ^= W1[46];
  Out[88] ^= W2[44];
  Out[89] ^= W2[45];
  Out[90] ^= W2[46];
  Out[110] ^= W3[44];
  Out[111] ^= W3[45];
}
#else
#define gfmul_66 karat2_66
static inline void karat2_66(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[66], D1[66], D2[66], SAA[33], SBB[33];
    gfmul_33(D0, A, B);
    gfmul_33(D2, (A+33), (B+33));
    for(int32_t i = 0; i < 33; i++) {
        int32_t is = i + 33;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
    gfmul_33(D1, SAA, SBB);
    for(int32_t i = 0; i < 33; i++) {
        int32_t is = i + 33;
        int32_t is2 = is + 33;
        int32_t is3 = is2 + 33;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
}
#endif
#define gfmul_67 tc3_67
static inline void tc3_67(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
  const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
  static uint32_t W0[46], W1[47], W2[48], W3[48], W4[42], tmp[48];
  U0 = &A[0];
  U1 = &A[23];
  U2 = &A[46];
  V0 = &B[0];
  V1 = &B[23];
  V2 = &B[46];
  for (int32_t i = 0 ; i < 21 ; i++) {
      W3[i] = U0[i] ^ U1[i] ^ U2[i];
      W2[i] = V0[i] ^ V1[i] ^ V2[i];
  }
  W3[21] = U0[21] ^ U1[21];
  W2[21] = V0[21] ^ V1[21];
  W3[22] = U0[22] ^ U1[22];
  W2[22] = V0[22] ^ V1[22];
  gfmul_23(W1, W2, W3);
  W0[0] = 0;
  W4[0] = 0;
  W0[1] = U1[0];
  W4[1] = V1[0];
  for (int32_t i = 1 ; i < 22 ; i++) {
      W0[i + 1] = U1[i] ^ U2[i - 1];
      W4[i + 1] = V1[i] ^ V2[i - 1];
  }

  W0[23] = U1[22];
  W4[23] = V1[22];
  for (int32_t i = 0 ; i < 23 ; i++) {
      W3[i] ^= W0[i];
      W2[i] ^= W4[i];
  }
  W3[23] = W0[23];
  W2[23] = W4[23];
  for (int32_t i = 0 ; i < 23 ; i++) {
      W0[i] ^= U0[i];
      W4[i] ^= V0[i];
  }
  gfmul_24(tmp, W3, W2);
  for (int32_t i = 0 ; i < 48 ; i++) {
      W3[i] = tmp[i];
  }
  gfmul_24(W2, W0, W4);
  gfmul_21(W4, U2, V2);
  gfmul_23(W0, U0, V0);
  for (int32_t i = 0 ; i < 48 ; i++) {
      W3[i] ^= W2[i];
  }
  for (int32_t i = 0 ; i < 46 ; i++) {
      W1[i] ^= W0[i];
  }
  for (int32_t i = 0 ; i < 45 ; i++) {
      int32_t i1 = i + 1;
      W2[i] = W2[i1] ^ W0[i1];
  }
  W2[45] = W2[46];
  W2[46] = W2[47];
  for (int32_t i = 0 ; i < 42 ; i++) {
      tmp[i] = W2[i] ^ W3[i] ^ W4[i];
  }
  for (int32_t i = 42 ; i < 47 ; i++) {
      tmp[i] = W2[i] ^ W3[i];
  }
  tmp[47] = W3[47];
  for (int32_t i = 0 ; i < 42 ; i++) {
      tmp[i + 3] ^= W4[i];
  }
  divide_by_x_plus_one(tmp, W2, 48);
  for (int32_t i = 0 ; i < 45 ; i++) {
      int32_t i1 = i + 1;
      tmp[i] = W3[i1] ^ W1[i1];
  }
  tmp[45] = W3[46];
  tmp[46] = W3[47];
  divide_by_x_plus_one(tmp, W3, 47);
  for (int32_t i = 0 ; i < 42 ; i++) {
      W1[i] ^= W2[i] ^ W4[i];
  }
  for (int32_t i = 42 ; i < 46 ; i++) {
      W1[i] ^= W2[i];
  }
  W1[46] = W2[46];
  for (int32_t i = 0 ; i < 46 ; i++) {
      W2[i] ^= W3[i];
  }
  for (int32_t i = 0; i < 19; i++) {
      int32_t j = i + 23;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 23] = W1[j] ^ W2[i];
      Out[j + 46] = W2[j] ^ W3[i];
      Out[i + 92] = W3[j] ^ W4[i];
      Out[j + 92] = W4[j];
  }
  for (int32_t i = 19; i < 23; i++) {
      int32_t j = i + 23;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 23] = W1[j] ^ W2[i];
      Out[j + 46] = W2[j] ^ W3[i];
      Out[i + 92] = W3[j] ^ W4[i];
  }
  Out[69] ^= W1[46];
  Out[92] ^= W2[46];
}

#define gfmul_68 tc3_68
static inline void tc3_68(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
  const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
      static uint32_t W0[46], W1[47], W2[48], W3[48], W4[44], tmp[48];
  U0 = &A[0];
  U1 = &A[23];
  U2 = &A[46];
  V0 = &B[0];
  V1 = &B[23];
  V2 = &B[46];
  for (int32_t i = 0 ; i < 22 ; i++) {
      W3[i] = U0[i] ^ U1[i] ^ U2[i];
      W2[i] = V0[i] ^ V1[i] ^ V2[i];
  }
  W3[22] = U0[22] ^ U1[22];
  W2[22] = V0[22] ^ V1[22];
  gfmul_23(W1, W2, W3);
  W0[0] = 0;
  W4[0] = 0;
  W0[1] = U1[0];
  W4[1] = V1[0];
  for (int32_t i = 1 ; i < 23 ; i++) {
      W0[i + 1] = U1[i] ^ U2[i - 1];
      W4[i + 1] = V1[i] ^ V2[i - 1];
  }
  for (int32_t i = 0 ; i < 23 ; i++) {
      W3[i] ^= W0[i];
      W2[i] ^= W4[i];
  }
  W3[23] = W0[23];
  W2[23] = W4[23];
  for (int32_t i = 0 ; i < 23 ; i++) {
      W0[i] ^= U0[i];
      W4[i] ^= V0[i];
  }
  gfmul_24(tmp, W3, W2);
  for (int32_t i = 0 ; i < 48 ; i++) {
      W3[i] = tmp[i];
  }
  gfmul_24(W2, W0, W4);
  gfmul_22(W4, U2, V2);
  gfmul_23(W0, U0, V0);
  for (int32_t i = 0 ; i < 48 ; i++) {
      W3[i] ^= W2[i];
  }
  for (int32_t i = 0 ; i < 46 ; i++) {
      W1[i] ^= W0[i];
  }
  for (int32_t i = 0 ; i < 45 ; i++) {
      int32_t i1 = i + 1;
      W2[i] = W2[i1] ^ W0[i1];
  }
  W2[45] = W2[46];
  W2[46] = W2[47];
  for (int32_t i = 0 ; i < 44 ; i++) {
      tmp[i] = W2[i] ^ W3[i] ^ W4[i];
  }
  for (int32_t i = 44 ; i < 47 ; i++) {
      tmp[i] = W2[i] ^ W3[i];
  }
  tmp[47] = W3[47];
  for (int32_t i = 0 ; i < 44 ; i++) {
      tmp[i + 3] ^= W4[i];
  }
  divide_by_x_plus_one(tmp, W2, 48);
  for (int32_t i = 0 ; i < 45 ; i++) {
      int32_t i1 = i + 1;
      tmp[i] = W3[i1] ^ W1[i1];
  }
  tmp[45] = W3[46];
  tmp[46] = W3[47];
  divide_by_x_plus_one(tmp, W3, 47);
  for (int32_t i = 0 ; i < 44 ; i++) {
      W1[i] ^= W2[i] ^ W4[i];
  }
  for (int32_t i = 44 ; i < 46 ; i++) {
      W1[i] ^= W2[i];
  }
  W1[46] = W2[46];
  for (int32_t i = 0 ; i < 46 ; i++) {
      W2[i] ^= W3[i];
  }
  for (int32_t i = 0; i < 21; i++) {
      int32_t j = i + 23;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 23] = W1[j] ^ W2[i];
      Out[j + 46] = W2[j] ^ W3[i];
      Out[i + 92] = W3[j] ^ W4[i];
      Out[j + 92] = W4[j];
  }
  for (int32_t i = 21; i < 23; i++) {
      int32_t j = i + 23;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 23] = W1[j] ^ W2[i];
      Out[j + 46] = W2[j] ^ W3[i];
      Out[i + 92] = W3[j] ^ W4[i];
  }
  Out[69] ^= W1[46];
  Out[92] ^= W2[46];
}

#define gfmul_69 tc3_69
static inline void tc3_69(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
  const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
  static uint32_t W0[46], W1[49], W2[50], W3[50], W4[46], tmp[50];
  U0 = &A[0];
  U1 = &A[23];
  U2 = &A[46];
  V0 = &B[0];
  V1 = &B[23];
  V2 = &B[46];
  for (int32_t i = 0 ; i < 23 ; i++) {
      W3[i] = U0[i] ^ U1[i] ^ U2[i];
      W2[i] = V0[i] ^ V1[i] ^ V2[i];
  }
  gfmul_23(W1, W2, W3);
  W0[0] = 0;
  W4[0] = 0;
  W0[1] = U1[0];
  W4[1] = V1[0];
  for (int32_t i = 1 ; i < 23 ; i++) {
      W0[i + 1] = U1[i] ^ U2[i - 1];
      W4[i + 1] = V1[i] ^ V2[i - 1];
  }
  W0[24] = U2[22];
  W4[24] = V2[22];
  for (int32_t i = 0 ; i < 23 ; i++) {
      W3[i] ^= W0[i];
      W2[i] ^= W4[i];
  }
  W3[23] = W0[23];
  W3[24] = W0[24];
  W2[23] = W4[23];
  W2[24] = W4[24];
  for (int32_t i = 0 ; i < 23 ; i++) {
      W0[i] ^= U0[i];
      W4[i] ^= V0[i];
  }
  gfmul_25(tmp, W3, W2);
  for (int32_t i = 0 ; i < 50 ; i++) {
      W3[i] = tmp[i];
  }
  gfmul_25(W2, W0, W4);
  gfmul_23(W4, U2, V2);
  gfmul_23(W0, U0, V0);
  for (int32_t i = 0 ; i < 50 ; i++) {
      W3[i] ^= W2[i];
  }
  for (int32_t i = 0 ; i < 46 ; i++) {
      W1[i] ^= W0[i];
  }
  for (int32_t i = 0 ; i < 45 ; i++) {
      int32_t i1 = i + 1;
      W2[i] = W2[i1] ^ W0[i1];
  }
  W2[45] = W2[46];
  W2[46] = W2[47];
  W2[47] = W2[48];
  W2[48] = W2[49];
  for (int32_t i = 0 ; i < 46 ; i++) {
      tmp[i] = W2[i] ^ W3[i] ^ W4[i];
  }
  for (int32_t i = 46 ; i < 49 ; i++) {
      tmp[i] = W2[i] ^ W3[i];
  }
  tmp[49] = W3[49];
  for (int32_t i = 0 ; i < 46 ; i++) {
      tmp[i + 3] ^= W4[i];
  }
  divide_by_x_plus_one(tmp, W2, 50);
  for (int32_t i = 0 ; i < 45 ; i++) {
      int32_t i1 = i + 1;
      tmp[i] = W3[i1] ^ W1[i1];
  }
  tmp[45] = W3[46];
  tmp[46] = W3[47];
  tmp[47] = W3[48];
  tmp[48] = W3[49];
  divide_by_x_plus_one(tmp, W3, 49);
  for (int32_t i = 0 ; i < 46 ; i++) {
      W1[i] ^= W2[i] ^ W4[i];
  }
  for (int32_t i = 46 ; i < 49 ; i++) {
      W1[i] = W2[i];
  }
  for (int32_t i = 0 ; i < 48 ; i++) {
      W2[i] ^= W3[i];
  }
  for (int32_t i = 0; i < 23; i++) {
      int32_t j = i + 23;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 23] = W1[j] ^ W2[i];
      Out[j + 46] = W2[j] ^ W3[i];
      Out[i + 92] = W3[j] ^ W4[i];
      Out[j + 92] = W4[j];
  }
  Out[69] ^= W1[46];
  Out[70] ^= W1[47];
  Out[71] ^= W1[48];
  Out[92] ^= W2[46];
  Out[93] ^= W2[47];
  Out[94] ^= W2[48];
  Out[115] ^= W3[46];
  Out[116] ^= W3[47];
}

#define gfmul_199 tc3_199
static inline void tc3_199(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
  const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
  static uint32_t W0[134], W1[135], W2[136], W3[136], W4[130], tmp[136];
  U0 = &A[0];
  U1 = &A[67];
  U2 = &A[134];
  V0 = &B[0];
  V1 = &B[67];
  V2 = &B[134];
  for (int32_t i = 0 ; i < 65 ; i++) {
      W3[i] = U0[i] ^ U1[i] ^ U2[i];
      W2[i] = V0[i] ^ V1[i] ^ V2[i];
  }
  W3[65] = U0[65] ^ U1[65];
  W2[65] = V0[65] ^ V1[65];
  W3[66] = U0[66] ^ U1[66];
  W2[66] = V0[66] ^ V1[66];
  gfmul_67(W1, W2, W3);
  W0[0] = 0;
  W4[0] = 0;
  W0[1] = U1[0];
  W4[1] = V1[0];
  for (int32_t i = 1 ; i < 66 ; i++) {
      W0[i + 1] = U1[i] ^ U2[i - 1];
      W4[i + 1] = V1[i] ^ V2[i - 1];
  }

  W0[67] = U1[66];
  W4[67] = V1[66];
  for (int32_t i = 0 ; i < 67 ; i++) {
      W3[i] ^= W0[i];
      W2[i] ^= W4[i];
  }
  W3[67] = W0[67];
  W2[67] = W4[67];
  for (int32_t i = 0 ; i < 67 ; i++) {
      W0[i] ^= U0[i];
      W4[i] ^= V0[i];
  }
  gfmul_68(tmp, W3, W2);
  for (int32_t i = 0 ; i < 136 ; i++) {
      W3[i] = tmp[i];
  }
  gfmul_68(W2, W0, W4);
  gfmul_65(W4, U2, V2);
  gfmul_67(W0, U0, V0);
  for (int32_t i = 0 ; i < 136 ; i++) {
      W3[i] ^= W2[i];
  }
  for (int32_t i = 0 ; i < 134 ; i++) {
      W1[i] ^= W0[i];
  }
  for (int32_t i = 0 ; i < 133 ; i++) {
      int32_t i1 = i + 1;
      W2[i] = W2[i1] ^ W0[i1];
  }
  W2[133] = W2[134];
  W2[134] = W2[135];
  for (int32_t i = 0 ; i < 130 ; i++) {
      tmp[i] = W2[i] ^ W3[i] ^ W4[i];
  }
  for (int32_t i = 130 ; i < 135 ; i++) {
      tmp[i] = W2[i] ^ W3[i];
  }
  tmp[135] = W3[135];
  for (int32_t i = 0 ; i < 130 ; i++) {
      tmp[i + 3] ^= W4[i];
  }
  divide_by_x_plus_one(tmp, W2, 136);
  for (int32_t i = 0 ; i < 133 ; i++) {
      int32_t i1 = i + 1;
      tmp[i] = W3[i1] ^ W1[i1];
  }
  tmp[133] = W3[134];
  tmp[134] = W3[135];
  divide_by_x_plus_one(tmp, W3, 135);
  for (int32_t i = 0 ; i < 130 ; i++) {
      W1[i] ^= W2[i] ^ W4[i];
  }
  for (int32_t i = 130 ; i < 134 ; i++) {
      W1[i] ^= W2[i];
  }
  W1[134] = W2[134];
  for (int32_t i = 0 ; i < 134 ; i++) {
      W2[i] ^= W3[i];
  }
  for (int32_t i = 0; i < 63; i++) {
      int32_t j = i + 67;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 67] = W1[j] ^ W2[i];
      Out[j + 134] = W2[j] ^ W3[i];
      Out[i + 268] = W3[j] ^ W4[i];
      Out[j + 268] = W4[j];
  }
  for (int32_t i = 63; i < 67; i++) {
      int32_t j = i + 67;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 67] = W1[j] ^ W2[i];
      Out[j + 134] = W2[j] ^ W3[i];
      Out[i + 268] = W3[j] ^ W4[i];
  }
  Out[201] ^= W1[134];
  Out[268] ^= W2[134];
}

#define gfmul_200 tc3_200
static inline void tc3_200(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
  const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
      static uint32_t W0[134], W1[135], W2[136], W3[136], W4[132], tmp[136];
  U0 = &A[0];
  U1 = &A[67];
  U2 = &A[134];
  V0 = &B[0];
  V1 = &B[67];
  V2 = &B[134];
  for (int32_t i = 0 ; i < 66 ; i++) {
      W3[i] = U0[i] ^ U1[i] ^ U2[i];
      W2[i] = V0[i] ^ V1[i] ^ V2[i];
  }
  W3[66] = U0[66] ^ U1[66];
  W2[66] = V0[66] ^ V1[66];
  gfmul_67(W1, W2, W3);
  W0[0] = 0;
  W4[0] = 0;
  W0[1] = U1[0];
  W4[1] = V1[0];
  for (int32_t i = 1 ; i < 67 ; i++) {
      W0[i + 1] = U1[i] ^ U2[i - 1];
      W4[i + 1] = V1[i] ^ V2[i - 1];
  }
  for (int32_t i = 0 ; i < 67 ; i++) {
      W3[i] ^= W0[i];
      W2[i] ^= W4[i];
  }
  W3[67] = W0[67];
  W2[67] = W4[67];
  for (int32_t i = 0 ; i < 67 ; i++) {
      W0[i] ^= U0[i];
      W4[i] ^= V0[i];
  }
  gfmul_68(tmp, W3, W2);
  for (int32_t i = 0 ; i < 136 ; i++) {
      W3[i] = tmp[i];
  }
  gfmul_68(W2, W0, W4);
  gfmul_66(W4, U2, V2);
  gfmul_67(W0, U0, V0);
  for (int32_t i = 0 ; i < 136 ; i++) {
      W3[i] ^= W2[i];
  }
  for (int32_t i = 0 ; i < 134 ; i++) {
      W1[i] ^= W0[i];
  }
  for (int32_t i = 0 ; i < 133 ; i++) {
      int32_t i1 = i + 1;
      W2[i] = W2[i1] ^ W0[i1];
  }
  W2[133] = W2[134];
  W2[134] = W2[135];
  for (int32_t i = 0 ; i < 132 ; i++) {
      tmp[i] = W2[i] ^ W3[i] ^ W4[i];
  }
  for (int32_t i = 132 ; i < 135 ; i++) {
      tmp[i] = W2[i] ^ W3[i];
  }
  tmp[135] = W3[135];
  for (int32_t i = 0 ; i < 132 ; i++) {
      tmp[i + 3] ^= W4[i];
  }
  divide_by_x_plus_one(tmp, W2, 136);
  for (int32_t i = 0 ; i < 133 ; i++) {
      int32_t i1 = i + 1;
      tmp[i] = W3[i1] ^ W1[i1];
  }
  tmp[133] = W3[134];
  tmp[134] = W3[135];
  divide_by_x_plus_one(tmp, W3, 135);
  for (int32_t i = 0 ; i < 132 ; i++) {
      W1[i] ^= W2[i] ^ W4[i];
  }
  for (int32_t i = 132 ; i < 134 ; i++) {
      W1[i] ^= W2[i];
  }
  W1[134] = W2[134];
  for (int32_t i = 0 ; i < 134 ; i++) {
      W2[i] ^= W3[i];
  }
  for (int32_t i = 0; i < 65; i++) {
      int32_t j = i + 67;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 67] = W1[j] ^ W2[i];
      Out[j + 134] = W2[j] ^ W3[i];
      Out[i + 268] = W3[j] ^ W4[i];
      Out[j + 268] = W4[j];
  }
  for (int32_t i = 65; i < 67; i++) {
      int32_t j = i + 67;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 67] = W1[j] ^ W2[i];
      Out[j + 134] = W2[j] ^ W3[i];
      Out[i + 268] = W3[j] ^ W4[i];
  }
  Out[201] ^= W1[134];
  Out[268] ^= W2[134];
}

#define gfmul_201 tc3_201
static inline void tc3_201(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
  const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
  static uint32_t W0[134], W1[137], W2[138], W3[138], W4[134], tmp[138];
  U0 = &A[0];
  U1 = &A[67];
  U2 = &A[134];
  V0 = &B[0];
  V1 = &B[67];
  V2 = &B[134];
  for (int32_t i = 0 ; i < 67 ; i++) {
      W3[i] = U0[i] ^ U1[i] ^ U2[i];
      W2[i] = V0[i] ^ V1[i] ^ V2[i];
  }
  gfmul_67(W1, W2, W3);
  W0[0] = 0;
  W4[0] = 0;
  W0[1] = U1[0];
  W4[1] = V1[0];
  for (int32_t i = 1 ; i < 67 ; i++) {
      W0[i + 1] = U1[i] ^ U2[i - 1];
      W4[i + 1] = V1[i] ^ V2[i - 1];
  }
  W0[68] = U2[66];
  W4[68] = V2[66];
  for (int32_t i = 0 ; i < 67 ; i++) {
      W3[i] ^= W0[i];
      W2[i] ^= W4[i];
  }
  W3[67] = W0[67];
  W3[68] = W0[68];
  W2[67] = W4[67];
  W2[68] = W4[68];
  for (int32_t i = 0 ; i < 67 ; i++) {
      W0[i] ^= U0[i];
      W4[i] ^= V0[i];
  }
  gfmul_69(tmp, W3, W2);
  for (int32_t i = 0 ; i < 138 ; i++) {
      W3[i] = tmp[i];
  }
  gfmul_69(W2, W0, W4);
  gfmul_67(W4, U2, V2);
  gfmul_67(W0, U0, V0);
  for (int32_t i = 0 ; i < 138 ; i++) {
      W3[i] ^= W2[i];
  }
  for (int32_t i = 0 ; i < 134 ; i++) {
      W1[i] ^= W0[i];
  }
  for (int32_t i = 0 ; i < 133 ; i++) {
      int32_t i1 = i + 1;
      W2[i] = W2[i1] ^ W0[i1];
  }
  W2[133] = W2[134];
  W2[134] = W2[135];
  W2[135] = W2[136];
  W2[136] = W2[137];
  for (int32_t i = 0 ; i < 134 ; i++) {
      tmp[i] = W2[i] ^ W3[i] ^ W4[i];
  }
  for (int32_t i = 134 ; i < 137 ; i++) {
      tmp[i] = W2[i] ^ W3[i];
  }
  tmp[137] = W3[137];
  for (int32_t i = 0 ; i < 134 ; i++) {
      tmp[i + 3] ^= W4[i];
  }
  divide_by_x_plus_one(tmp, W2, 138);
  for (int32_t i = 0 ; i < 133 ; i++) {
      int32_t i1 = i + 1;
      tmp[i] = W3[i1] ^ W1[i1];
  }
  tmp[133] = W3[134];
  tmp[134] = W3[135];
  tmp[135] = W3[136];
  tmp[136] = W3[137];
  divide_by_x_plus_one(tmp, W3, 137);
  for (int32_t i = 0 ; i < 134 ; i++) {
      W1[i] ^= W2[i] ^ W4[i];
  }
  for (int32_t i = 134 ; i < 137 ; i++) {
      W1[i] = W2[i];
  }
  for (int32_t i = 0 ; i < 136 ; i++) {
      W2[i] ^= W3[i];
  }
  for (int32_t i = 0; i < 67; i++) {
      int32_t j = i + 67;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 67] = W1[j] ^ W2[i];
      Out[j + 134] = W2[j] ^ W3[i];
      Out[i + 268] = W3[j] ^ W4[i];
      Out[j + 268] = W4[j];
  }
  Out[201] ^= W1[134];
  Out[202] ^= W1[135];
  Out[203] ^= W1[136];
  Out[268] ^= W2[134];
  Out[269] ^= W2[135];
  Out[270] ^= W2[136];
  Out[335] ^= W3[134];
  Out[336] ^= W3[135];
}

#define gfmul_202 tc3_202
static inline void tc3_202(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
  const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
  static uint32_t W0[136], W1[137], W2[138], W3[138], W4[132], tmp[138];
  U0 = &A[0];
  U1 = &A[68];
  U2 = &A[136];
  V0 = &B[0];
  V1 = &B[68];
  V2 = &B[136];
  for (int32_t i = 0 ; i < 66 ; i++) {
      W3[i] = U0[i] ^ U1[i] ^ U2[i];
      W2[i] = V0[i] ^ V1[i] ^ V2[i];
  }
  W3[66] = U0[66] ^ U1[66];
  W2[66] = V0[66] ^ V1[66];
  W3[67] = U0[67] ^ U1[67];
  W2[67] = V0[67] ^ V1[67];
  gfmul_68(W1, W2, W3);
  W0[0] = 0;
  W4[0] = 0;
  W0[1] = U1[0];
  W4[1] = V1[0];
  for (int32_t i = 1 ; i < 67 ; i++) {
      W0[i + 1] = U1[i] ^ U2[i - 1];
      W4[i + 1] = V1[i] ^ V2[i - 1];
  }

  W0[68] = U1[67];
  W4[68] = V1[67];
  for (int32_t i = 0 ; i < 68 ; i++) {
      W3[i] ^= W0[i];
      W2[i] ^= W4[i];
  }
  W3[68] = W0[68];
  W2[68] = W4[68];
  for (int32_t i = 0 ; i < 68 ; i++) {
      W0[i] ^= U0[i];
      W4[i] ^= V0[i];
  }
  gfmul_69(tmp, W3, W2);
  for (int32_t i = 0 ; i < 138 ; i++) {
      W3[i] = tmp[i];
  }
  gfmul_69(W2, W0, W4);
  gfmul_66(W4, U2, V2);
  gfmul_68(W0, U0, V0);
  for (int32_t i = 0 ; i < 138 ; i++) {
      W3[i] ^= W2[i];
  }
  for (int32_t i = 0 ; i < 136 ; i++) {
      W1[i] ^= W0[i];
  }
  for (int32_t i = 0 ; i < 135 ; i++) {
      int32_t i1 = i + 1;
      W2[i] = W2[i1] ^ W0[i1];
  }
  W2[135] = W2[136];
  W2[136] = W2[137];
  for (int32_t i = 0 ; i < 132 ; i++) {
      tmp[i] = W2[i] ^ W3[i] ^ W4[i];
  }
  for (int32_t i = 132 ; i < 137 ; i++) {
      tmp[i] = W2[i] ^ W3[i];
  }
  tmp[137] = W3[137];
  for (int32_t i = 0 ; i < 132 ; i++) {
      tmp[i + 3] ^= W4[i];
  }
  divide_by_x_plus_one(tmp, W2, 138);
  for (int32_t i = 0 ; i < 135 ; i++) {
      int32_t i1 = i + 1;
      tmp[i] = W3[i1] ^ W1[i1];
  }
  tmp[135] = W3[136];
  tmp[136] = W3[137];
  divide_by_x_plus_one(tmp, W3, 137);
  for (int32_t i = 0 ; i < 132 ; i++) {
      W1[i] ^= W2[i] ^ W4[i];
  }
  for (int32_t i = 132 ; i < 136 ; i++) {
      W1[i] ^= W2[i];
  }
  W1[136] = W2[136];
  for (int32_t i = 0 ; i < 136 ; i++) {
      W2[i] ^= W3[i];
  }
  for (int32_t i = 0; i < 64; i++) {
      int32_t j = i + 68;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 68] = W1[j] ^ W2[i];
      Out[j + 136] = W2[j] ^ W3[i];
      Out[i + 272] = W3[j] ^ W4[i];
      Out[j + 272] = W4[j];
  }
  for (int32_t i = 64; i < 68; i++) {
      int32_t j = i + 68;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 68] = W1[j] ^ W2[i];
      Out[j + 136] = W2[j] ^ W3[i];
      Out[i + 272] = W3[j] ^ W4[i];
  }
  Out[204] ^= W1[136];
  Out[272] ^= W2[136];
}

#define gfmul_600 tc3_600
static inline void tc3_600(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
  const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
  static uint32_t W0[400], W1[403], W2[404], W3[404], W4[400], tmp[404];
  U0 = &A[0];
  U1 = &A[200];
  U2 = &A[400];
  V0 = &B[0];
  V1 = &B[200];
  V2 = &B[400];
  for (int32_t i = 0 ; i < 200 ; i++) {
      W3[i] = U0[i] ^ U1[i] ^ U2[i];
      W2[i] = V0[i] ^ V1[i] ^ V2[i];
  }
  gfmul_200(W1, W2, W3);
  W0[0] = 0;
  W4[0] = 0;
  W0[1] = U1[0];
  W4[1] = V1[0];
  for (int32_t i = 1 ; i < 200 ; i++) {
      W0[i + 1] = U1[i] ^ U2[i - 1];
      W4[i + 1] = V1[i] ^ V2[i - 1];
  }
  W0[201] = U2[199];
  W4[201] = V2[199];
  for (int32_t i = 0 ; i < 200 ; i++) {
      W3[i] ^= W0[i];
      W2[i] ^= W4[i];
  }
  W3[200] = W0[200];
  W3[201] = W0[201];
  W2[200] = W4[200];
  W2[201] = W4[201];
  for (int32_t i = 0 ; i < 200 ; i++) {
      W0[i] ^= U0[i];
      W4[i] ^= V0[i];
  }
  gfmul_202(tmp, W3, W2);
  for (int32_t i = 0 ; i < 404 ; i++) {
      W3[i] = tmp[i];
  }
  gfmul_202(W2, W0, W4);
  gfmul_200(W4, U2, V2);
  gfmul_200(W0, U0, V0);
  for (int32_t i = 0 ; i < 404 ; i++) {
      W3[i] ^= W2[i];
  }
  for (int32_t i = 0 ; i < 400 ; i++) {
      W1[i] ^= W0[i];
  }
  for (int32_t i = 0 ; i < 399 ; i++) {
      int32_t i1 = i + 1;
      W2[i] = W2[i1] ^ W0[i1];
  }
  W2[399] = W2[400];
  W2[400] = W2[401];
  W2[401] = W2[402];
  W2[402] = W2[403];
  for (int32_t i = 0 ; i < 400 ; i++) {
      tmp[i] = W2[i] ^ W3[i] ^ W4[i];
  }
  for (int32_t i = 400 ; i < 403 ; i++) {
      tmp[i] = W2[i] ^ W3[i];
  }
  tmp[403] = W3[403];
  for (int32_t i = 0 ; i < 400 ; i++) {
      tmp[i + 3] ^= W4[i];
  }
  divide_by_x_plus_one(tmp, W2, 404);
  for (int32_t i = 0 ; i < 399 ; i++) {
      int32_t i1 = i + 1;
      tmp[i] = W3[i1] ^ W1[i1];
  }
  tmp[399] = W3[400];
  tmp[400] = W3[401];
  tmp[401] = W3[402];
  tmp[402] = W3[403];
  divide_by_x_plus_one(tmp, W3, 403);
  for (int32_t i = 0 ; i < 400 ; i++) {
      W1[i] ^= W2[i] ^ W4[i];
  }
  for (int32_t i = 400 ; i < 403 ; i++) {
      W1[i] = W2[i];
  }
  for (int32_t i = 0 ; i < 402 ; i++) {
      W2[i] ^= W3[i];
  }
  for (int32_t i = 0; i < 200; i++) {
      int32_t j = i + 200;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 200] = W1[j] ^ W2[i];
      Out[j + 400] = W2[j] ^ W3[i];
      Out[i + 800] = W3[j] ^ W4[i];
      Out[j + 800] = W4[j];
  }
  Out[600] ^= W1[400];
  Out[601] ^= W1[401];
  Out[602] ^= W1[402];
  Out[800] ^= W2[400];
  Out[801] ^= W2[401];
  Out[802] ^= W2[402];
  Out[1000] ^= W3[400];
  Out[1001] ^= W3[401];
}

#define gfmul_601 tc3_601
static inline void tc3_601(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
  const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
  static uint32_t W0[402], W1[403], W2[404], W3[404], W4[398], tmp[404];
  U0 = &A[0];
  U1 = &A[201];
  U2 = &A[402];
  V0 = &B[0];
  V1 = &B[201];
  V2 = &B[402];
  for (int32_t i = 0 ; i < 199 ; i++) {
      W3[i] = U0[i] ^ U1[i] ^ U2[i];
      W2[i] = V0[i] ^ V1[i] ^ V2[i];
  }
  W3[199] = U0[199] ^ U1[199];
  W2[199] = V0[199] ^ V1[199];
  W3[200] = U0[200] ^ U1[200];
  W2[200] = V0[200] ^ V1[200];
  gfmul_201(W1, W2, W3);
  W0[0] = 0;
  W4[0] = 0;
  W0[1] = U1[0];
  W4[1] = V1[0];
  for (int32_t i = 1 ; i < 200 ; i++) {
      W0[i + 1] = U1[i] ^ U2[i - 1];
      W4[i + 1] = V1[i] ^ V2[i - 1];
  }

  W0[201] = U1[200];
  W4[201] = V1[200];
  for (int32_t i = 0 ; i < 201 ; i++) {
      W3[i] ^= W0[i];
      W2[i] ^= W4[i];
  }
  W3[201] = W0[201];
  W2[201] = W4[201];
  for (int32_t i = 0 ; i < 201 ; i++) {
      W0[i] ^= U0[i];
      W4[i] ^= V0[i];
  }
  gfmul_202(tmp, W3, W2);
  for (int32_t i = 0 ; i < 404 ; i++) {
      W3[i] = tmp[i];
  }
  gfmul_202(W2, W0, W4);
  gfmul_199(W4, U2, V2);
  gfmul_201(W0, U0, V0);
  for (int32_t i = 0 ; i < 404 ; i++) {
      W3[i] ^= W2[i];
  }
  for (int32_t i = 0 ; i < 402 ; i++) {
      W1[i] ^= W0[i];
  }
  for (int32_t i = 0 ; i < 401 ; i++) {
      int32_t i1 = i + 1;
      W2[i] = W2[i1] ^ W0[i1];
  }
  W2[401] = W2[402];
  W2[402] = W2[403];
  for (int32_t i = 0 ; i < 398 ; i++) {
      tmp[i] = W2[i] ^ W3[i] ^ W4[i];
  }
  for (int32_t i = 398 ; i < 403 ; i++) {
      tmp[i] = W2[i] ^ W3[i];
  }
  tmp[403] = W3[403];
  for (int32_t i = 0 ; i < 398 ; i++) {
      tmp[i + 3] ^= W4[i];
  }
  divide_by_x_plus_one(tmp, W2, 404);
  for (int32_t i = 0 ; i < 401 ; i++) {
      int32_t i1 = i + 1;
      tmp[i] = W3[i1] ^ W1[i1];
  }
  tmp[401] = W3[402];
  tmp[402] = W3[403];
  divide_by_x_plus_one(tmp, W3, 403);
  for (int32_t i = 0 ; i < 398 ; i++) {
      W1[i] ^= W2[i] ^ W4[i];
  }
  for (int32_t i = 398 ; i < 402 ; i++) {
      W1[i] ^= W2[i];
  }
  W1[402] = W2[402];
  for (int32_t i = 0 ; i < 402 ; i++) {
      W2[i] ^= W3[i];
  }
  for (int32_t i = 0; i < 197; i++) {
      int32_t j = i + 201;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 201] = W1[j] ^ W2[i];
      Out[j + 402] = W2[j] ^ W3[i];
      Out[i + 804] = W3[j] ^ W4[i];
      Out[j + 804] = W4[j];
  }
  for (int32_t i = 197; i < 201; i++) {
      int32_t j = i + 201;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 201] = W1[j] ^ W2[i];
      Out[j + 402] = W2[j] ^ W3[i];
      Out[i + 804] = W3[j] ^ W4[i];
  }
  Out[603] ^= W1[402];
  Out[804] ^= W2[402];
}

#define gfmul_602 tc3_602
static inline void tc3_602(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
  const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
      static uint32_t W0[402], W1[403], W2[404], W3[404], W4[400], tmp[404];
  U0 = &A[0];
  U1 = &A[201];
  U2 = &A[402];
  V0 = &B[0];
  V1 = &B[201];
  V2 = &B[402];
  for (int32_t i = 0 ; i < 200 ; i++) {
      W3[i] = U0[i] ^ U1[i] ^ U2[i];
      W2[i] = V0[i] ^ V1[i] ^ V2[i];
  }
  W3[200] = U0[200] ^ U1[200];
  W2[200] = V0[200] ^ V1[200];
  gfmul_201(W1, W2, W3);
  W0[0] = 0;
  W4[0] = 0;
  W0[1] = U1[0];
  W4[1] = V1[0];
  for (int32_t i = 1 ; i < 201 ; i++) {
      W0[i + 1] = U1[i] ^ U2[i - 1];
      W4[i + 1] = V1[i] ^ V2[i - 1];
  }
  for (int32_t i = 0 ; i < 201 ; i++) {
      W3[i] ^= W0[i];
      W2[i] ^= W4[i];
  }
  W3[201] = W0[201];
  W2[201] = W4[201];
  for (int32_t i = 0 ; i < 201 ; i++) {
      W0[i] ^= U0[i];
      W4[i] ^= V0[i];
  }
  gfmul_202(tmp, W3, W2);
  for (int32_t i = 0 ; i < 404 ; i++) {
      W3[i] = tmp[i];
  }
  gfmul_202(W2, W0, W4);
  gfmul_200(W4, U2, V2);
  gfmul_201(W0, U0, V0);
  for (int32_t i = 0 ; i < 404 ; i++) {
      W3[i] ^= W2[i];
  }
  for (int32_t i = 0 ; i < 402 ; i++) {
      W1[i] ^= W0[i];
  }
  for (int32_t i = 0 ; i < 401 ; i++) {
      int32_t i1 = i + 1;
      W2[i] = W2[i1] ^ W0[i1];
  }
  W2[401] = W2[402];
  W2[402] = W2[403];
  for (int32_t i = 0 ; i < 400 ; i++) {
      tmp[i] = W2[i] ^ W3[i] ^ W4[i];
  }
  for (int32_t i = 400 ; i < 403 ; i++) {
      tmp[i] = W2[i] ^ W3[i];
  }
  tmp[403] = W3[403];
  for (int32_t i = 0 ; i < 400 ; i++) {
      tmp[i + 3] ^= W4[i];
  }
  divide_by_x_plus_one(tmp, W2, 404);
  for (int32_t i = 0 ; i < 401 ; i++) {
      int32_t i1 = i + 1;
      tmp[i] = W3[i1] ^ W1[i1];
  }
  tmp[401] = W3[402];
  tmp[402] = W3[403];
  divide_by_x_plus_one(tmp, W3, 403);
  for (int32_t i = 0 ; i < 400 ; i++) {
      W1[i] ^= W2[i] ^ W4[i];
  }
  for (int32_t i = 400 ; i < 402 ; i++) {
      W1[i] ^= W2[i];
  }
  W1[402] = W2[402];
  for (int32_t i = 0 ; i < 402 ; i++) {
      W2[i] ^= W3[i];
  }
  for (int32_t i = 0; i < 199; i++) {
      int32_t j = i + 201;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 201] = W1[j] ^ W2[i];
      Out[j + 402] = W2[j] ^ W3[i];
      Out[i + 804] = W3[j] ^ W4[i];
      Out[j + 804] = W4[j];
  }
  for (int32_t i = 199; i < 201; i++) {
      int32_t j = i + 201;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 201] = W1[j] ^ W2[i];
      Out[j + 402] = W2[j] ^ W3[i];
      Out[i + 804] = W3[j] ^ W4[i];
  }
  Out[603] ^= W1[402];
  Out[804] ^= W2[402];
}

#define gfmul_1802 tc3_1802
static inline void tc3_1802(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
  const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
  static uint32_t W0[1202], W1[1203], W2[1204], W3[1204], W4[1200], tmp[1204];
  U0 = &A[0];
  U1 = &A[601];
  U2 = &A[1202];
  V0 = &B[0];
  V1 = &B[601];
  V2 = &B[1202];
  for (int32_t i = 0 ; i < 600 ; i++) {
      W3[i] = U0[i] ^ U1[i] ^ U2[i];
      W2[i] = V0[i] ^ V1[i] ^ V2[i];
  }
  W3[600] = U0[600] ^ U1[600];
  W2[600] = V0[600] ^ V1[600];
  gfmul_601(W1, W2, W3);
  W0[0] = 0;
  W4[0] = 0;
  W0[1] = U1[0];
  W4[1] = V1[0];
  for (int32_t i = 1 ; i < 601 ; i++) {
      W0[i + 1] = U1[i] ^ U2[i - 1];
      W4[i + 1] = V1[i] ^ V2[i - 1];
  }
  for (int32_t i = 0 ; i < 601 ; i++) {
      W3[i] ^= W0[i];
      W2[i] ^= W4[i];
  }
  W3[601] = W0[601];
  W2[601] = W4[601];
  for (int32_t i = 0 ; i < 601 ; i++) {
      W0[i] ^= U0[i];
      W4[i] ^= V0[i];
  }
  gfmul_602(tmp, W3, W2);
  for (int32_t i = 0 ; i < 1204 ; i++) {
      W3[i] = tmp[i];
  }
  gfmul_602(W2, W0, W4);
  gfmul_600(W4, U2, V2);
  gfmul_601(W0, U0, V0);
  for (int32_t i = 0 ; i < 1204 ; i++) {
      W3[i] ^= W2[i];
  }
  for (int32_t i = 0 ; i < 1202 ; i++) {
      W1[i] ^= W0[i];
  }
  for (int32_t i = 0 ; i < 1201 ; i++) {
      int32_t i1 = i + 1;
      W2[i] = W2[i1] ^ W0[i1];
  }
  W2[1201] = W2[1202];
  W2[1202] = W2[1203];
  for (int32_t i = 0 ; i < 1200 ; i++) {
      tmp[i] = W2[i] ^ W3[i] ^ W4[i];
  }
  for (int32_t i = 1200 ; i < 1203 ; i++) {
      tmp[i] = W2[i] ^ W3[i];
  }
  tmp[1203] = W3[1203];
  for (int32_t i = 0 ; i < 1200 ; i++) {
      tmp[i + 3] ^= W4[i];
  }
  divide_by_x_plus_one(tmp, W2, 1204);
  for (int32_t i = 0 ; i < 1201 ; i++) {
      int32_t i1 = i + 1;
      tmp[i] = W3[i1] ^ W1[i1];
  }
  tmp[1201] = W3[1202];
  tmp[1202] = W3[1203];
  divide_by_x_plus_one(tmp, W3, 1203);
  for (int32_t i = 0 ; i < 1200 ; i++) {
      W1[i] ^= W2[i] ^ W4[i];
  }
  for (int32_t i = 1200 ; i < 1202 ; i++) {
      W1[i] ^= W2[i];
  }
  W1[1202] = W2[1202];
  for (int32_t i = 0 ; i < 1202 ; i++) {
      W2[i] ^= W3[i];
  }
  for (int32_t i = 0; i < 599; i++) {
      int32_t j = i + 601;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 601] = W1[j] ^ W2[i];
      Out[j + 1202] = W2[j] ^ W3[i];
      Out[i + 2404] = W3[j] ^ W4[i];
      Out[j + 2404] = W4[j];
  }
  for (int32_t i = 599; i < 601; i++) {
      int32_t j = i + 601;
      Out[i] = W0[i];
      Out[j] = W0[j] ^ W1[i];
      Out[j + 601] = W1[j] ^ W2[i];
      Out[j + 1202] = W2[j] ^ W3[i];
      Out[i + 2404] = W3[j] ^ W4[i];
  }
  Out[1803] ^= W1[1202];
  Out[2404] ^= W2[1202];
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
void PQCLEAN_HQC256_CLEAN_vect_mul(uint64_t *o, const uint64_t *v1, const uint64_t *v2) {

    uint32_t stk[1802<<1];
	uint32_t *sa = stk, *sb = stk + 1802;
    
	poly_swap(sa, (uint32_t*)v1, 1802);
	poly_swap(sb, (uint32_t*)v2, 1802);
	gfmul_1802(stk, sa, sb);
	poly_invswap(stk, 2*1802);

	reduce((uint32_t*)o, (uint32_t*)stk);
}
