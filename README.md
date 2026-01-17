# Efficient Polynomial Multiplication for HQC on ARM Cortex-M4

<<<<<<< HEAD
=======
## Authors

- Jihoon Jang, Myeonghoon Lee, Donggeun Kwon, Seokhie Hong, Suhri Kim, Sangjin Lee

## License

All implementations are in the public domain.

>>>>>>> f84ee1d175eb15598bb92c984868620c9a62bde9
## Contents

- **HQC_opt_pqm4**: Implementation of HQC on Cortex-M4, which can be integrated into [PQM4](https://github.com/mupq/pqm4).

- **KaratTC_tree_generator**: Generator for Karatsuba/Toom-Cook combination trees.

## Instructions for benchmarking

Our implementation targets the NUCLEO-L4R5ZI (STM32L4R5ZI) board. 

### Integrate to pqm4

First, download the pqm4 library:

```bash
git clone --recursive https://github.com/mupq/pqm4.git
```

<<<<<<< HEAD
Then, copy the contents of
`HQC_opt_pqm4/FAFFT_LJKH25`,
`HQC_opt_pqm4/FAFFT_butterfly_opt`,
`HQC_opt_pqm4/hybrid_FAFFT-CRT`,
`HQC_opt_pqm4/hybrid_FAFFT-Karatsuba`,
`HQC_opt_pqm4/radix16`
 into `pqm4/crypto_kem`:

```bash
cp -r HQC_opt_pqm4/FAFFT_LJKH25/* pqm4/crypto_kem/
cp -r HQC_opt_pqm4/FAFFT_butterfly_opt/* pqm4/crypto_kem/
cp -r HQC_opt_pqm4/hybrid_FAFFT-CRT/* pqm4/crypto_kem/
=======
Then, copy the contents of `HQC_opt_pqm4/hybrid_FAFFT-Karatsuba` and `HQC_opt_pqm4/radix-16` into `pqm4/crypto_kem`:

```bash
>>>>>>> f84ee1d175eb15598bb92c984868620c9a62bde9
cp -r HQC_opt_pqm4/hybrid_FAFFT-Karatsuba/* pqm4/crypto_kem/
cp -r HQC_opt_pqm4/radix-16/* pqm4/crypto_kem/
cd pqm4
```

<<<<<<< HEAD
From this point on, follow the standard workflow of the pqm4 framework.
=======
From this point on, follow the standard workflow of the pqm4 framework.

## References

Our implementation is based on the following works:

- For the HQC implementation, we utilized code from *"Improved Frobenius FFT for Code-Based Cryptography on Cortex-M4"* by Myeonghoon Lee et al., IEEE Internet of Things Journal, 2025.
  The corresponding code is available at: [https://github.com/myhoon/FAFFT_HQC](https://github.com/myhoon/FAFFT_HQC)
>>>>>>> f84ee1d175eb15598bb92c984868620c9a62bde9
