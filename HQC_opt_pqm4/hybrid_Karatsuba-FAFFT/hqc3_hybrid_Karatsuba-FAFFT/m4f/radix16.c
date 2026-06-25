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

#define gfmul_10 karat2_10
static inline void karat2_10(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    static uint32_t D0[10], D1[10], D2[10], SAA[5], SBB[5];
    gfmul_5(D0, A, B);
    gfmul_5(D2, (A+5), (B+5));
    for(int32_t i = 0; i < 5; i++) {
        int32_t is = i + 5;
        SAA[i] = A[i] ^ A[is];
        SBB[i] = B[i] ^ B[is];
    }
    gfmul_5(D1, SAA, SBB);
    for(int32_t i = 0; i < 5; i++) {
        int32_t is = i + 5;
        int32_t is2 = is + 5;
        int32_t is3 = is2 + 5;
        uint32_t middle = D0[is] ^ D2[i];
        Out[i]   = D0[i];
        Out[is]  = middle ^ D0[i] ^ D1[i];
        Out[is2] = middle ^ D1[is] ^ D2[is];
        Out[is3] = D2[is];
    }
}

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

#define gfmul_34 tc3_34
static inline void tc3_34(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[24], W1[25], W2[26], W3[26], W4[20], tmp[26];
    U0 = &A[0];
    U1 = &A[12];
    U2 = &A[24];
    V0 = &B[0];
    V1 = &B[12];
    V2 = &B[24];
    for (int32_t i = 0 ; i < 10 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[10] = U0[10] ^ U1[10];
    W2[10] = V0[10] ^ V1[10];
    W3[11] = U0[11] ^ U1[11];
    W2[11] = V0[11] ^ V1[11];
    gfmul_12(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 11 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    
    W0[12] = U1[11];
    W4[12] = V1[11];
    for (int32_t i = 0 ; i < 12 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[12] = W0[12];
    W2[12] = W4[12];
    for (int32_t i = 0 ; i < 12 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_13(tmp, W3, W2);
    for (int32_t i = 0 ; i < 26 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_13(W2, W0, W4);
    gfmul_10(W4, U2, V2);
    gfmul_12(W0, U0, V0);
    for (int32_t i = 0 ; i < 26 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 24 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 23 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[23] = W2[24];
    W2[24] = W2[25];
    for (int32_t i = 0 ; i < 20 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 20 ; i < 25 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[25] = W3[25];
    for (int32_t i = 0 ; i < 20 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 26);
    for (int32_t i = 0 ; i < 23 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[23] = W3[24];
    tmp[24] = W3[25];
    divide_by_x_plus_one(tmp, W3, 25);
    for (int32_t i = 0 ; i < 20 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 20 ; i < 24 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[24] = W2[24];
    for (int32_t i = 0 ; i < 24 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 8; i++) {
        int32_t j = i + 12;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 12] = W1[j] ^ W2[i];
        Out[j + 24] = W2[j] ^ W3[i];
        Out[i + 48] = W3[j] ^ W4[i];
        Out[j + 48] = W4[j];
    }
    for (int32_t i = 8; i < 12; i++) {
        int32_t j = i + 12;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 12] = W1[j] ^ W2[i];
        Out[j + 24] = W2[j] ^ W3[i];
        Out[i + 48] = W3[j] ^ W4[i];
    }
    Out[36] ^= W1[24];
    Out[48] ^= W2[24];
}

#define gfmul_97 tc3_97
static inline void tc3_97(uint32_t *Out,  const uint32_t *A,  const uint32_t *B){
    const uint32_t *U0, *U1, *U2, *V0, *V1, *V2;
    static uint32_t W0[66], W1[67], W2[68], W3[68], W4[62], tmp[68];
    U0 = &A[0];
    U1 = &A[33];
    U2 = &A[66];
    V0 = &B[0];
    V1 = &B[33];
    V2 = &B[66];
    for (int32_t i = 0 ; i < 31 ; i++) {
        W3[i] = U0[i] ^ U1[i] ^ U2[i];
        W2[i] = V0[i] ^ V1[i] ^ V2[i];
    }
    W3[31] = U0[31] ^ U1[31];
    W2[31] = V0[31] ^ V1[31];
    W3[32] = U0[32] ^ U1[32];
    W2[32] = V0[32] ^ V1[32];
    gfmul_33(W1, W2, W3);
    W0[0] = 0;
    W4[0] = 0;
    W0[1] = U1[0];
    W4[1] = V1[0];
    for (int32_t i = 1 ; i < 32 ; i++) {
        W0[i + 1] = U1[i] ^ U2[i - 1];
        W4[i + 1] = V1[i] ^ V2[i - 1];
    }
    
    W0[33] = U1[32];
    W4[33] = V1[32];
    for (int32_t i = 0 ; i < 33 ; i++) {
        W3[i] ^= W0[i];
        W2[i] ^= W4[i];
    }
    W3[33] = W0[33];
    W2[33] = W4[33];
    for (int32_t i = 0 ; i < 33 ; i++) {
        W0[i] ^= U0[i];
        W4[i] ^= V0[i];
    }
    gfmul_34(tmp, W3, W2);
    for (int32_t i = 0 ; i < 68 ; i++) {
        W3[i] = tmp[i];
    }
    gfmul_34(W2, W0, W4);
    gfmul_31(W4, U2, V2);
    gfmul_33(W0, U0, V0);
    for (int32_t i = 0 ; i < 68 ; i++) {
        W3[i] ^= W2[i];
    }
    for (int32_t i = 0 ; i < 66 ; i++) {
        W1[i] ^= W0[i];
    }
    for (int32_t i = 0 ; i < 65 ; i++) {
        int32_t i1 = i + 1;
        W2[i] = W2[i1] ^ W0[i1];
    }
    W2[65] = W2[66];
    W2[66] = W2[67];
    for (int32_t i = 0 ; i < 62 ; i++) {
        tmp[i] = W2[i] ^ W3[i] ^ W4[i];
    }
    for (int32_t i = 62 ; i < 67 ; i++) {
        tmp[i] = W2[i] ^ W3[i];
    }
    tmp[67] = W3[67];
    for (int32_t i = 0 ; i < 62 ; i++) {
        tmp[i + 3] ^= W4[i];
    }
    divide_by_x_plus_one(tmp, W2, 68);
    for (int32_t i = 0 ; i < 65 ; i++) {
        int32_t i1 = i + 1;
        tmp[i] = W3[i1] ^ W1[i1];
    }
    tmp[65] = W3[66];
    tmp[66] = W3[67];
    divide_by_x_plus_one(tmp, W3, 67);
    for (int32_t i = 0 ; i < 62 ; i++) {
        W1[i] ^= W2[i] ^ W4[i];
    }
    for (int32_t i = 62 ; i < 66 ; i++) {
        W1[i] ^= W2[i];
    }
    W1[66] = W2[66];
    for (int32_t i = 0 ; i < 66 ; i++) {
        W2[i] ^= W3[i];
    }
    for (int32_t i = 0; i < 29; i++) {
        int32_t j = i + 33;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 33] = W1[j] ^ W2[i];
        Out[j + 66] = W2[j] ^ W3[i];
        Out[i + 132] = W3[j] ^ W4[i];
        Out[j + 132] = W4[j];
    }
    for (int32_t i = 29; i < 33; i++) {
        int32_t j = i + 33;
        Out[i] = W0[i];
        Out[j] = W0[j] ^ W1[i];
        Out[j + 33] = W1[j] ^ W2[i];
        Out[j + 66] = W2[j] ^ W3[i];
        Out[i + 132] = W3[j] ^ W4[i];
    }
    Out[99] ^= W1[66];
    Out[132] ^= W2[66];
}

void radix16_97(uint32_t *o, const uint32_t *a, const uint32_t *b)
{
	uint32_t sa[97], sb[97];
	poly_swap(sa, a, 97);
    poly_swap(sb, b, 97);
    gfmul_97(o, sa, sb);
    poly_invswap(o, 2*97);
}
