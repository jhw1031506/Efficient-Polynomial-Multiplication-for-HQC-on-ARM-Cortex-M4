// Implemented by Jihoon Jang and Myeonghoon Lee
#include "gf2x.h"
#include "parameters.h"
#include <stddef.h>
#include <stdint.h>
#include "reduce.h"
#include <string.h>

//CASE can be 1-4
#ifndef CASE
    #define CASE 1
#endif

#if CASE==1 //default
#define WMUL 1
#elif CASE==2
#define WMUL 2
#elif CASE==3
#define WMUL 3
#elif CASE==4
#define WMUL 4
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

#define gfmul_2 mul2
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
// warning #13200: No EMMS instruction before return
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
// warning #13200: No EMMS instruction before return
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
// warning #13200: No EMMS instruction before return
}
#if WMUL!=1
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

#if WMUL==3 || WMUL ==4
#define gfmul_19 tc3_19
static inline void tc3_19(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[14], W1[15], W2[16], W3[16], W4[10], tmp[16];
    U0 = &A[0];
    U1 = &A[7];
    U2 = &A[14];
    V0 = &B[0];
    V1 = &B[7];
    V2 = &B[14];
    for (int32_t i = 0 ; i < 5 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[5] = U0[5] ^ U1[5];
    W2[5] = V0[5] ^ V1[5];
    W3[6] = U0[6] ^ U1[6];
    W2[6] = V0[6] ^ V1[6];
    gfmul_7(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 6 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    
    W0[7] = U1[6];
    W4[7] = V1[6];
    for (int32_t i = 0 ; i < 7 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[7] = W0[7];
    W2[7] = W4[7];
    for (int32_t i = 0 ; i < 7 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_8(tmp, W3, W2);
    for (int32_t i = 0 ; i < 16 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_8(W2, W0, W4);
    gfmul_5(W4, U2, V2);
    gfmul_7(W0, U0, V0);
    for (int32_t i = 0 ; i < 16 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 14 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 13 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[13] = W2[14];
    W2[14] = W2[15];
    for (int32_t i = 0 ; i < 10 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 10 ; i < 15 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[15] = W3[15];
    for (int32_t i = 0 ; i < 10 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 16);
    for (int32_t i = 0 ; i < 13 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[13] = W3[14];
    tmp[14] = W3[15];
    divide_by_x_plus_one(tmp, W3, 15);
    for (int32_t i = 0 ; i < 10 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 10 ; i < 14 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[14] = W2[14];
    for (int32_t i = 0 ; i < 14 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 3; i++) {
        int32_t j = i + 7;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 7] = W1[j] ^ W2[i];
        Out[j + 14] = W2[j] ^ W3[i];
        Out[i + 28] = W3[j] ^ W4[i];
        Out[j + 28] = W4[j];
    }
    for (int32_t i = 3; i < 7; i++) {
        int32_t j = i + 7;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 7] = W1[j] ^ W2[i];
        Out[j + 14] = W2[j] ^ W3[i];
        Out[i + 28] = W3[j] ^ W4[i];
    }
    Out[21] ^= W1[14];
    Out[28] ^= W2[14];
}
#endif

#if WMUL!=1
#define gfmul_20 tc3_20
static inline void tc3_20(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
        static uint32_t W0[14], W1[15], W2[16], W3[16], W4[12], tmp[16];
    U0 = &A[0];
    U1 = &A[7];
    U2 = &A[14];
    V0 = &B[0];
    V1 = &B[7];
    V2 = &B[14];
    for (int32_t i = 0 ; i < 6 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[6] = U0[6] ^ U1[6];
    W2[6] = V0[6] ^ V1[6];
    gfmul_7(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 7 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    for (int32_t i = 0 ; i < 7 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[7] = W0[7];
    W2[7] = W4[7];
    for (int32_t i = 0 ; i < 7 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_8(tmp, W3, W2);
    for (int32_t i = 0 ; i < 16 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_8(W2, W0, W4);
    gfmul_6(W4, U2, V2);
    gfmul_7(W0, U0, V0);
    for (int32_t i = 0 ; i < 16 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 14 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 13 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[13] = W2[14];
    W2[14] = W2[15];
    for (int32_t i = 0 ; i < 12 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 12 ; i < 15 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[15] = W3[15];
    for (int32_t i = 0 ; i < 12 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 16);
    for (int32_t i = 0 ; i < 13 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[13] = W3[14];
    tmp[14] = W3[15];
    divide_by_x_plus_one(tmp, W3, 15);
    for (int32_t i = 0 ; i < 12 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 12 ; i < 14 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[14] = W2[14];
    for (int32_t i = 0 ; i < 14 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 5; i++) {
        int32_t j = i + 7;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 7] = W1[j] ^ W2[i];
        Out[j + 14] = W2[j] ^ W3[i];
        Out[i + 28] = W3[j] ^ W4[i];
        Out[j + 28] = W4[j];
    }
    for (int32_t i = 5; i < 7; i++) {
        int32_t j = i + 7;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 7] = W1[j] ^ W2[i];
        Out[j + 14] = W2[j] ^ W3[i];
        Out[i + 28] = W3[j] ^ W4[i];
    }
    Out[21] ^= W1[14];
    Out[28] ^= W2[14];
}
#endif

#if WMUL!=1
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
#endif

#if WMUL!=1
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
#endif

#if WMUL==4
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
#endif

#if WMUL == 2 || WMUL == 1
#define gfmul_30 karat2_30
static inline void karat2_30(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
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

#define gfmul_31 karat2_31
static inline void karat2_31(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
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
#if WMUL == 1 || WMUL == 2
#define gfmul_61 karat2_61
static inline void karat2_61(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
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
#else
#define gfmul_61 tc3_61
static inline void tc3_61(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[42], W1[43], W2[44], W3[44], W4[38], tmp[44];
    U0 = &A[0];
    U1 = &A[21];
    U2 = &A[42];
    V0 = &B[0];
    V1 = &B[21];
    V2 = &B[42];
    for (int32_t i = 0 ; i < 19 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[19] = U0[19] ^ U1[19];
    W2[19] = V0[19] ^ V1[19];
    W3[20] = U0[20] ^ U1[20];
    W2[20] = V0[20] ^ V1[20];
    gfmul_21(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 20 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    
    W0[21] = U1[20];
    W4[21] = V1[20];
    for (int32_t i = 0 ; i < 21 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[21] = W0[21];
    W2[21] = W4[21];
    for (int32_t i = 0 ; i < 21 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_22(tmp, W3, W2);
    for (int32_t i = 0 ; i < 44 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_22(W2, W0, W4);
    gfmul_19(W4, U2, V2);
    gfmul_21(W0, U0, V0);
    for (int32_t i = 0 ; i < 44 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 42 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 41 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[41] = W2[42];
    W2[42] = W2[43];
    for (int32_t i = 0 ; i < 38 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 38 ; i < 43 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[43] = W3[43];
    for (int32_t i = 0 ; i < 38 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 44);
    for (int32_t i = 0 ; i < 41 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[41] = W3[42];
    tmp[42] = W3[43];
    divide_by_x_plus_one(tmp, W3, 43);
    for (int32_t i = 0 ; i < 38 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 38 ; i < 42 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[42] = W2[42];
    for (int32_t i = 0 ; i < 42 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 17; i++) {
        int32_t j = i + 21;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 21] = W1[j] ^ W2[i];
        Out[j + 42] = W2[j] ^ W3[i];
        Out[i + 84] = W3[j] ^ W4[i];
        Out[j + 84] = W4[j];
    }
    for (int32_t i = 17; i < 21; i++) {
        int32_t j = i + 21;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 21] = W1[j] ^ W2[i];
        Out[j + 42] = W2[j] ^ W3[i];
        Out[i + 84] = W3[j] ^ W4[i];
    }
    Out[63] ^= W1[42];
    Out[84] ^= W2[42];
}
#endif
#if WMUL==1
#define gfmul_62 karat2_62
static inline void karat2_62(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
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
#else
#define gfmul_62 tc3_62
static inline void tc3_62(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
        static uint32_t W0[42], W1[43], W2[44], W3[44], W4[40], tmp[44];
    U0 = &A[0];
    U1 = &A[21];
    U2 = &A[42];
    V0 = &B[0];
    V1 = &B[21];
    V2 = &B[42];
    for (int32_t i = 0 ; i < 20 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[20] = U0[20] ^ U1[20];
    W2[20] = V0[20] ^ V1[20];
    gfmul_21(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 21 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    for (int32_t i = 0 ; i < 21 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[21] = W0[21];
    W2[21] = W4[21];
    for (int32_t i = 0 ; i < 21 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_22(tmp, W3, W2);
    for (int32_t i = 0 ; i < 44 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_22(W2, W0, W4);
    gfmul_20(W4, U2, V2);
    gfmul_21(W0, U0, V0);
    for (int32_t i = 0 ; i < 44 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 42 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 41 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[41] = W2[42];
    W2[42] = W2[43];
    for (int32_t i = 0 ; i < 40 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 40 ; i < 43 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[43] = W3[43];
    for (int32_t i = 0 ; i < 40 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 44);
    for (int32_t i = 0 ; i < 41 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[41] = W3[42];
    tmp[42] = W3[43];
    divide_by_x_plus_one(tmp, W3, 43);
    for (int32_t i = 0 ; i < 40 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 40 ; i < 42 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[42] = W2[42];
    for (int32_t i = 0 ; i < 42 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 19; i++) {
        int32_t j = i + 21;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 21] = W1[j] ^ W2[i];
        Out[j + 42] = W2[j] ^ W3[i];
        Out[i + 84] = W3[j] ^ W4[i];
        Out[j + 84] = W4[j];
    }
    for (int32_t i = 19; i < 21; i++) {
        int32_t j = i + 21;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 21] = W1[j] ^ W2[i];
        Out[j + 42] = W2[j] ^ W3[i];
        Out[i + 84] = W3[j] ^ W4[i];
    }
    Out[63] ^= W1[42];
    Out[84] ^= W2[42];
}
#endif

#define gfmul_63 karat2_63
static inline void karat2_63(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
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
#if WMUL == 1 || WMUL == 2 || WMUL == 3
#define gfmul_64 karat2_64
static inline void karat2_64(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
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
#else
#define gfmul_64 tc3_64
static inline void tc3_64(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[44], W1[45], W2[46], W3[46], W4[40], tmp[46];
    U0 = &A[0];
    U1 = &A[22];
    U2 = &A[44];
    V0 = &B[0];
    V1 = &B[22];
    V2 = &B[44];
    for (int32_t i = 0 ; i < 20 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[20] = U0[20] ^ U1[20];
    W2[20] = V0[20] ^ V1[20];
    W3[21] = U0[21] ^ U1[21];
    W2[21] = V0[21] ^ V1[21];
    gfmul_22(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 21 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    
    W0[22] = U1[21];
    W4[22] = V1[21];
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
    gfmul_20(W4, U2, V2);
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
    for (int32_t i = 0 ; i < 40 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 40 ; i < 45 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[45] = W3[45];
    for (int32_t i = 0 ; i < 40 ; i++) {
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
    for (int32_t i = 0 ; i < 40 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 40 ; i < 44 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[44] = W2[44];
    for (int32_t i = 0 ; i < 44 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 18; i++) {
        int32_t j = i + 22;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 22] = W1[j] ^ W2[i];
        Out[j + 44] = W2[j] ^ W3[i];
        Out[i + 88] = W3[j] ^ W4[i];
        Out[j + 88] = W4[j];
    }
    for (int32_t i = 18; i < 22; i++) {
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
#endif

#define gfmul_183 tc3_183
static inline void tc3_183(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[122], W1[125], W2[126], W3[126], W4[122], tmp[126];
    U0 = &A[0];
    U1 = &A[61];
    U2 = &A[122];
    V0 = &B[0];
    V1 = &B[61];
    V2 = &B[122];
    for (int32_t i = 0 ; i < 61 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    gfmul_61(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 61 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    W0[62] = U2[60];
    W4[62] = V2[60];
    for (int32_t i = 0 ; i < 61 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[61] = W0[61];
    W3[62] = W0[62];
    W2[61] = W4[61];
    W2[62] = W4[62];
    for (int32_t i = 0 ; i < 61 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_63(tmp, W3, W2);
    for (int32_t i = 0 ; i < 126 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_63(W2, W0, W4);
    gfmul_61(W4, U2, V2);
    gfmul_61(W0, U0, V0);
    for (int32_t i = 0 ; i < 126 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 122 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 121 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[121] = W2[122];
    W2[122] = W2[123];
    W2[123] = W2[124];
    W2[124] = W2[125];
    for (int32_t i = 0 ; i < 122 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 122 ; i < 125 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[125] = W3[125];
    for (int32_t i = 0 ; i < 122 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 126);
    for (int32_t i = 0 ; i < 121 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[121] = W3[122];
    tmp[122] = W3[123];
    tmp[123] = W3[124];
    tmp[124] = W3[125];
    divide_by_x_plus_one(tmp, W3, 125);
    for (int32_t i = 0 ; i < 122 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 122 ; i < 125 ; i++) {
        W1[i] = W2[i];
    }
    for (int32_t i = 0 ; i < 124 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 61; i++) {
        int32_t j = i + 61;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 61] = W1[j] ^ W2[i];
        Out[j + 122] = W2[j] ^ W3[i];
        Out[i + 244] = W3[j] ^ W4[i];
        Out[j + 244] = W4[j];
    }
    Out[183] ^= W1[122];
    Out[184] ^= W1[123];
    Out[185] ^= W1[124];
    Out[244] ^= W2[122];
    Out[245] ^= W2[123];
    Out[246] ^= W2[124];
    Out[305] ^= W3[122];
    Out[306] ^= W3[123];
}

#define gfmul_185 tc3_185
static inline void tc3_185(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
        static uint32_t W0[124], W1[125], W2[126], W3[126], W4[122], tmp[126];
    U0 = &A[0];
    U1 = &A[62];
    U2 = &A[124];
    V0 = &B[0];
    V1 = &B[62];
    V2 = &B[124];
    for (int32_t i = 0 ; i < 61 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[61] = U0[61] ^ U1[61];
    W2[61] = V0[61] ^ V1[61];
    gfmul_62(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 62 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    for (int32_t i = 0 ; i < 62 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[62] = W0[62];
    W2[62] = W4[62];
    for (int32_t i = 0 ; i < 62 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_63(tmp, W3, W2);
    for (int32_t i = 0 ; i < 126 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_63(W2, W0, W4);
    gfmul_61(W4, U2, V2);
    gfmul_62(W0, U0, V0);
    for (int32_t i = 0 ; i < 126 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 124 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 123 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[123] = W2[124];
    W2[124] = W2[125];
    for (int32_t i = 0 ; i < 122 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 122 ; i < 125 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[125] = W3[125];
    for (int32_t i = 0 ; i < 122 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 126);
    for (int32_t i = 0 ; i < 123 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[123] = W3[124];
    tmp[124] = W3[125];
    divide_by_x_plus_one(tmp, W3, 125);
    for (int32_t i = 0 ; i < 122 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 122 ; i < 124 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[124] = W2[124];
    for (int32_t i = 0 ; i < 124 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 60; i++) {
        int32_t j = i + 62;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 62] = W1[j] ^ W2[i];
        Out[j + 124] = W2[j] ^ W3[i];
        Out[i + 248] = W3[j] ^ W4[i];
        Out[j + 248] = W4[j];
    }
    for (int32_t i = 60; i < 62; i++) {
        int32_t j = i + 62;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 62] = W1[j] ^ W2[i];
        Out[j + 124] = W2[j] ^ W3[i];
        Out[i + 248] = W3[j] ^ W4[i];
    }
    Out[186] ^= W1[124];
    Out[248] ^= W2[124];
}

#define gfmul_186 tc3_186
static inline void tc3_186(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[124], W1[127], W2[128], W3[128], W4[124], tmp[128];
    U0 = &A[0];
    U1 = &A[62];
    U2 = &A[124];
    V0 = &B[0];
    V1 = &B[62];
    V2 = &B[124];
    for (int32_t i = 0 ; i < 62 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    gfmul_62(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 62 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    W0[63] = U2[61];
    W4[63] = V2[61];
    for (int32_t i = 0 ; i < 62 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[62] = W0[62];
    W3[63] = W0[63];
    W2[62] = W4[62];
    W2[63] = W4[63];
    for (int32_t i = 0 ; i < 62 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_64(tmp, W3, W2);
    for (int32_t i = 0 ; i < 128 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_64(W2, W0, W4);
    gfmul_62(W4, U2, V2);
    gfmul_62(W0, U0, V0);
    for (int32_t i = 0 ; i < 128 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 124 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 123 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[123] = W2[124];
    W2[124] = W2[125];
    W2[125] = W2[126];
    W2[126] = W2[127];
    for (int32_t i = 0 ; i < 124 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 124 ; i < 127 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[127] = W3[127];
    for (int32_t i = 0 ; i < 124 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 128);
    for (int32_t i = 0 ; i < 123 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[123] = W3[124];
    tmp[124] = W3[125];
    tmp[125] = W3[126];
    tmp[126] = W3[127];
    divide_by_x_plus_one(tmp, W3, 127);
    for (int32_t i = 0 ; i < 124 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 124 ; i < 127 ; i++) {
        W1[i] = W2[i];
    }
    for (int32_t i = 0 ; i < 126 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 62; i++) {
        int32_t j = i + 62;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 62] = W1[j] ^ W2[i];
        Out[j + 124] = W2[j] ^ W3[i];
        Out[i + 248] = W3[j] ^ W4[i];
        Out[j + 248] = W4[j];
    }
    Out[186] ^= W1[124];
    Out[187] ^= W1[125];
    Out[188] ^= W1[126];
    Out[248] ^= W2[124];
    Out[249] ^= W2[125];
    Out[250] ^= W2[126];
    Out[310] ^= W3[124];
    Out[311] ^= W3[125];
}

#define gfmul_553 tc3_553
static inline void tc3_553(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[370], W1[371], W2[372], W3[372], W4[366], tmp[372];
    U0 = &A[0];
    U1 = &A[185];
    U2 = &A[370];
    V0 = &B[0];
    V1 = &B[185];
    V2 = &B[370];
    for (int32_t i = 0 ; i < 183 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[183] = U0[183] ^ U1[183];
    W2[183] = V0[183] ^ V1[183];
    W3[184] = U0[184] ^ U1[184];
    W2[184] = V0[184] ^ V1[184];
    gfmul_185(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 184 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }

    W0[185] = U1[184];
    W4[185] = V1[184];
    for (int32_t i = 0 ; i < 185 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[185] = W0[185];
    W2[185] = W4[185];
    for (int32_t i = 0 ; i < 185 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_186(tmp, W3, W2);
    for (int32_t i = 0 ; i < 372 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_186(W2, W0, W4);
    gfmul_183(W4, U2, V2);
    gfmul_185(W0, U0, V0);
    for (int32_t i = 0 ; i < 372 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 370 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 369 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[369] = W2[370];
    W2[370] = W2[371];
    for (int32_t i = 0 ; i < 366 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 366 ; i < 371 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[371] = W3[371];
    for (int32_t i = 0 ; i < 366 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 372);
    for (int32_t i = 0 ; i < 369 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[369] = W3[370];
    tmp[370] = W3[371];
    divide_by_x_plus_one(tmp, W3, 371);
    for (int32_t i = 0 ; i < 366 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 366 ; i < 370 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[370] = W2[370];
    for (int32_t i = 0 ; i < 370 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 181; i++) {
        int32_t j = i + 185;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 185] = W1[j] ^ W2[i];
        Out[j + 370] = W2[j] ^ W3[i];
        Out[i + 740] = W3[j] ^ W4[i];
        Out[j + 740] = W4[j];
    }
    for (int32_t i = 181; i < 185; i++) {
        int32_t j = i + 185;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 185] = W1[j] ^ W2[i];
        Out[j + 370] = W2[j] ^ W3[i];
        Out[i + 740] = W3[j] ^ W4[i];
    }
    Out[555] ^= W1[370];
    Out[740] ^= W2[370];
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

void PQCLEAN_HQC128_CLEAN_vect_mul(uint64_t *o, const uint64_t *v1, const uint64_t *v2) {

	uint32_t stk[553<<1];
	uint32_t *sa = stk, *sb = stk + 553;
    
	poly_swap(sa, (uint32_t*)v1, 553);
	poly_swap(sb, (uint32_t*)v2, 553);
	gfmul_553(stk, sa, sb);
	poly_invswap(stk, 2*553);

	reduce((uint32_t*)o, (uint32_t*)stk);
}
