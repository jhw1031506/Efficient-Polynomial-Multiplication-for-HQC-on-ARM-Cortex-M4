#include "parameters.h"
#include "radix16.h"
#include "gf2x.h"
#include <stddef.h>
#include <stdint.h>


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
     uint32_t d0[2], d1[2], d2[2], d01[2], d02[2], d12[2];
  
     radix_16_basemul_asm(d0, a[0], b[0]);
     radix_16_basemul_asm(d1, a[1], b[1]);
     radix_16_basemul_asm(d2, a[2], b[2]);
     radix_16_basemul_asm(d01, a[0]^a[1], b[0]^b[1]);
     radix_16_basemul_asm(d02, a[0]^a[2], b[0]^b[2]);
     radix_16_basemul_asm(d12, a[1]^a[2], b[1]^b[2]);
  
  
     c[0] = d0[0];
     c[1] = d0[1] ^ d01[0] ^ d1[0] ^ d0[0];
     c[2] = d01[1] ^ d1[1] ^ d0[1] ^ d02[0] ^ d2[0] ^ d0[0] ^ d1[0];
     c[3] = d02[1] ^ d2[1] ^ d0[1] ^ d1[1] ^ d12[0] ^ d1[0] ^ d2[0];
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



void radix16_41(uint32_t *o, const uint32_t *a, const uint32_t *b)
{
	uint32_t sa[41], sb[41];
	poly_swap(sa, a, 41);
    poly_swap(sb, b, 41);
    gfmul_41(o, sa, sb);
    poly_invswap(o, 2*41);
}
