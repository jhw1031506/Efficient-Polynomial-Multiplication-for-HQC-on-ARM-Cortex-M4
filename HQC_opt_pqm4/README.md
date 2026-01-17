# HQC_opt_pqm4

This directory contains HQC implementations where various optimized polynomial multiplication techniques are applied for the ARM Cortex-M4.

## Directory Structure

- FAFFT_LJKH25: HQC implementation using the Frobenius Additive FFT (FAFFT) as described in "Improved Frobenius FFT for Code-Based Cryptography on Cortex-M4" by Lee et al. (2025).
- FAFFT_butterfly_opt: HQC implementation using FAFFT with an optimized butterfly algorithm.
- hybrid_FAFFT-CRT: HQC implementation utilizing a hybrid approach of FAFFT and the Chinese Remainder Theorem.
- hybrid_FAFFT-Karatsuba: HQC implementation utilizing a hybrid approach of FAFFT and the Karatsuba algorithm.
- radix16: HQC implementation using the radix-16 representation for polynomial multiplication.
