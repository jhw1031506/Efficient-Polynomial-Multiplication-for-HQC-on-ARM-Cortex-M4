#include "gf2x.h"
#include "parameters.h"
#include <stddef.h>
#include <stdint.h>
#include "reduce.h"
#include "gfmul_fft.h"
#include "radix16.h"
#include <string.h>

void karatsuba_and_fft(uint8_t *o, const uint8_t *a, const uint8_t *b) {

	//for HQC-192, n = 35851
	//(n+63)/64 = 561.15625 then uint64_t a[561], uint8_t a[4488]. but the only 35851-th degree is vaild.
	//n/8 = 4481.375 then a[4481], b[4481]
	//a = a_0 (0~4095 array) + a_1 (4096~4481 array)
	//b = b_0 (0~4095 array) + b_1 (4096~4481 array)

	uint8_t a0pa1b0pb1[8192];
	uint8_t *o1, *o2;
	const uint8_t *a1, *b1;
	a1 = a + 4096;
	b1 = b + 4096;
	o1 = o + 4096;
	o2 = o + 8192;

	//at here, a and b are both 32767-degree but output must be 65534 
	//we first multiply a0 * b0 using FAFFT
	bmul2_4096_to_8192(o, a, b);

	//second, a1 * b1 using radix-16 multiplication. 
	//array range is 385 based on 8-bit. then, it must be size = 97.
	radix16_97((uint32_t*)o2, (const uint32_t*)a1, (const uint32_t*)b1);

	uint8_t a0pa1[4096], b0pb1[4096];

	//(a0+a1)(b0+b1) -> similar as a0 * b0 using FAFFT
	//a1b1 -> 4096 ~ 4481 array, total 386 array

	memcpy(a0pa1, a, 4096);
	memcpy(b0pb1, b, 4096);
	for (int iter = 0; iter < 386; iter++)
	{
		a0pa1[iter] = a[iter] ^ a1[iter];
		b0pb1[iter] = b[iter] ^ b1[iter];
	}

	//third, (a0+a1) * (b0+b1) using FAFFT.
	bmul2_4096_to_8192(a0pa1b0pb1, a0pa1, b0pb1);
	//===================================last phase============================

	//current : o[0]~o[4095] are a0 * b0, o[4096]~o[4417] are a1 * b1, and o[2048]
	//we need to cummulative result of (a0+a1)(b0+b1)-a0b0-a1b1 at o[2048]~o[4417]
	//the final results must be under 17669*2 degree then, we only consider 35338.
	for (int iter = 0; iter < 772; iter++)
	{
		o2[iter] = o1[iter] ^ o2[iter];
		o1[iter] = o2[iter];
	}
	for (int iter = 0; iter < 4096; iter++)
	{
		o1[iter] = o[iter] ^ o1[iter] ^ a0pa1b0pb1[iter];
	}
	for (int iter = 4096; iter < 4868; iter++)
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
