#include "gf2x.h"
#include "parameters.h"
#include <stddef.h>
#include <stdint.h>
#include "reduce.h"
#include "gfmul_fft.h"
#include "radix16.h"
#include <string.h>

void karatsuba_and_fft(uint8_t *o, const uint8_t *a, const uint8_t *b) {

	//for HQC-128, n = 17669
	//(n+63)/64 = 277.0625 then uint64_t a[277], uint8_t a[2216]. but the only 17669-th degree is vaild.
	//n/8 = 2208.625 then a[2208], b[2208]
	//a = a_0 (0~2047 array) + a_1 (2048~2208 array)
	//b = b_0 (0~2047 array) + b_1 (2048~2208 array)

	uint8_t a0pa1b0pb1[4096];
	uint8_t *o1, *o2;
	const uint8_t *a1, *b1;
	a1 = a + 2048;
	b1 = b + 2048;
	o1 = o + 2048;
	o2 = o + 4096;

	//at here, a and b are both 16383-degree but output must be 32766 
	//we first multiply a0 * b0 using FAFFT
	bmul2_2048_to_4096(o, a, b);

	//second, a1 * b1 using radix-16 multiplication. 
	//array range is 161 based on 8-bit. then, it must be size = 41.
	radix16_41((uint32_t*)o2, (const uint32_t*)a1, (const uint32_t*)b1);

	uint8_t a0pa1[2048], b0pb1[2048];

	//(a0+a1)(b0+b1) -> similar as a0 * b0 using FAFFT
	//a1b1 -> 2048 ~ 2208 array, total 161 array

	memcpy(a0pa1, a, 2048);
	memcpy(b0pb1, b, 2048);
	for (int iter = 0; iter < 161; iter++)
	{
		a0pa1[iter] = a[iter] ^ a1[iter];
		b0pb1[iter] = b[iter] ^ b1[iter];
	}

	//third, (a0+a1) * (b0+b1) using FAFFT.
	bmul2_2048_to_4096(a0pa1b0pb1, a0pa1, b0pb1);
	//===================================last phase============================

	//current : o[0]~o[4095] are a0 * b0, o[4096]~o[4417] are a1 * b1, and o[2048]
	//we need to cummulative result of (a0+a1)(b0+b1)-a0b0-a1b1 at o[2048]~o[4417]
	//the final results must be under 17669*2 degree then, we only consider 35338.
	for (int iter = 0; iter < 322; iter++)
	{
		o2[iter] = o1[iter] ^ o2[iter];
		o1[iter] = o2[iter];
	}
	for (int iter = 0; iter < 2048; iter++)
	{
		o1[iter] = o[iter] ^ o1[iter] ^ a0pa1b0pb1[iter];
	}
	for (int iter = 2048; iter < 2370; iter++)
	{
		o1[iter] = o1[iter] ^ a0pa1b0pb1[iter];
	}
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
void vect_mul(uint64_t *o, const uint64_t *v1, const uint64_t *v2) {

	uint32_t o_u32[VEC_N_SIZE_32 << 1] = { 0 };
	karatsuba_and_fft((uint8_t*)o_u32, (uint8_t*)v1, (uint8_t*)v2);
	reduce((uint32_t*)o, (uint32_t*)o_u32);

}
