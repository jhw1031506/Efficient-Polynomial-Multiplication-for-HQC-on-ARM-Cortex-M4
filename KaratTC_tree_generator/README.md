# Karatsuba/Toom-Cook algorithm combination generator
This program explores the Karatsuba/Toom-Cook algorithm for GF(2) polynomial multiplication and builds the optimal recursion tree under a linear cost model:
```bash
total_cost = (#base_muls) * w_mul + (#XORs)
```

w_mul is a relative multiplication weight scaled by 100 (e.g., 1.75 → 175).
Dynamic programming computes the minimal cost for each length and weight, records the first split algorithm, and prints the resulting tree and stats.

## Cost Model & Methods

Scale: All costs are stored ×100 so (w_mul) and XOR counts add directly.

Methods considered: Karatsuba and Toom-Cook variants (including typical multi-way splits as applicable).

Search range: w_mul ∈ [1.00, 499.99] (internally 100..49999).

## Build
```bash
gcc -o KaratTC_tree_gen KaratTC_tree_gen.c
```

## Configure

Macros (set via -D or in-source):

MODE (default 1): 1=HQC-128, 2=HQC-192, 3=HQC-256

COMPARE_WITH_GF2XLIB (default 0): 1=comparison mode for gf2x

Example:

```bash
gcc -DMODE=3 -DCOMPARE_WITH_GF2XLIB=1 -o KaratTC_tree_gen KaratTC_tree_gen.c
```

## Run
```bash
./KaratTC_tree_gen
```