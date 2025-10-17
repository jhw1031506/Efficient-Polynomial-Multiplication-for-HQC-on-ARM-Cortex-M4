#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*
 *  - For GF(2) polynomial multiplication using divide-and-conquer schemes
 *    (Karatsuba 2/3/5-way and Toom-Cook 3-way), this program finds the optimal
 *    recursive multiplication tree that minimizes:
 *        total_cost = (number_of_basemuls) * w_mul + (number_of_XORs)
 *    where w_mul is the relative cost (scaled by 100).
 *  - For each weight w and length len (1..POLY_LENGTH), dynamic programming
 *    computes the minimal total cost w_min[w][len] and records the first split
 *    algorithm first_alg[w][len] that achieves it.
 *  - For the top length (POLY_LENGTH), the code prints the chosen tree and
 *    accumulates statistics on internal subcalls per sub-length.
 *
 *  - Cost model: totals are stored at scale ×100 so that (w_mul) and XOR counts
 *    can be summed directly.
 *  - Algorithms: KARAT2, KARAT3, KARAT5, TC3 (Toom-Cook 3-way). Each has different
 *    child-length patterns and recursion multiplicities.
 *  - diff_tree_weight[] stores breakpoints of w where the optimal tree structure
 *    (i.e., the set of sub-lengths that appear) changes.
 *  - print_call_stats() traverses the chosen tree and tallies how many times
 *    each sub-length is called, plus the total internal call count.
 */
#ifndef min
    #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif


#define CEIL_DIVIDE(a, b)  (((a)/(b)) + ((a) % (b) == 0 ? 0 : 1)) // Divide a by b and ceil the result

#ifndef MODE
    #define MODE 1       //1 : hqc-128, 2 : hqc-192, 3 : hqc-256
#endif

#ifndef COMPARE_WITH_GF2XLIB
    #define COMPARE_WITH_GF2XLIB 0 //1: restrict choices for comparison with gf2x (karat2, tc3 only)
#endif

#define HQC128 17669
#define HQC192 35851
#define HQC256 57637

#if MODE==1
#define N_HQC ((HQC128+31)/32)*256
#elif MODE==2
#define N_HQC ((HQC192+31)/32)*256
#elif MODE==3
#define N_HQC ((HQC256+31)/32)*256
#else
#define N_HQC 1285
#endif

#define KARAT2 0
#define KARAT3 1
#define KARAT5 2
#define TC3    3

#define MAX_W 50000         // evaluate weights w in [100, MAX_W-1] i.e., 1.00 to 499.99
#define POLY_LENGTH (CEIL_DIVIDE(N_HQC,256)) // top polynomial length (ceil to 256-blocks)

 /*
  * DP tables and metadata
  *  - w_min[w][len]: minimal total cost at weight=w and length=len (scaled ×100)
  *  - first_alg[w][len]: first split algorithm chosen at (w,len) (KARAT2/3/5/TC3)
  *  - length_before[w][]: unique sub-lengths that appear in the tree for weight w
  *  - firstalg_before[w][]: companion array for the first algorithm at those lengths
  *  - diff_tree_weight[]: increasing list of weights where the optimal tree set of
  *    sub-lengths changes (structure breakpoints)
  */
uint64_t w_min[MAX_W][POLY_LENGTH + 1];     // w_min[i]   : minimal cost for length i
uint64_t first_alg[MAX_W][POLY_LENGTH + 1]; // first_alg[i] : first chosen algorithm for length i
uint64_t length_before[MAX_W][100] = { 0 };
uint64_t firstalg_before[MAX_W][100] = { 0 };
int length_before_len[MAX_W] = { 0 };
uint64_t diff_tree_weight[MAX_W] = { 0 };
uint64_t diff_tree_w_len = 0;

/* Pair sorting utility: sort (A[],B[]) by A in descending order (B follows) */
typedef struct {
    int a;
    int b;
} Pair;

/* qsort comparator: descending by a */
static int cmp_pair(const void* x, const void* y) {
    const Pair* p = (const Pair*)x;
    const Pair* q = (const Pair*)y;
    if (p->a < q->a) return 1;
    if (p->a > q->a) return -1;
    return 0;
}

/* Sort A as key and permute B accordingly (no-op if allocation fails) */
void sort_with_companion(uint64_t* A, uint64_t* B, size_t n) {
    Pair* tmp = malloc(n * sizeof(Pair));
    if (!tmp) return; 

    for (size_t i = 0; i < n; ++i) {
        tmp[i].a = A[i];
        tmp[i].b = B[i];
    }

    qsort(tmp, n, sizeof(Pair), cmp_pair);

    for (size_t i = 0; i < n; ++i) {
        A[i] = tmp[i].a;
        B[i] = tmp[i].b;
    }

    free(tmp);
}

/* Helper to show (A,B) after companion sort: debugging/reporting */
void sort_print(uint64_t* A, uint64_t* B, size_t len) {

    uint64_t A_sort[100] = { 0 };
    uint64_t B_sort[100] = { 0 };

    memcpy(B_sort, B, len * 8);
    memcpy(A_sort, A, len * 8);
    sort_with_companion(A_sort, B_sort, len);
    for (int j = 0; j < len; j++) {
        printf("%3ld %3ld\n", A_sort[j], B_sort[j]);
    }
}



/* Return 1 if arrays equal over len elements, else 0 */
int8_t arr_compare(uint64_t* A, uint64_t* B, int len) {
    for (uint16_t i = 0; i < len; i++) {
        if (A[i] != B[i]) return 0;
    }
    return 1;
}

/*
 * xor_cnt(len, alg): per-split XOR cost at length=len for the given algorithm.
 * Return value is scaled by 100 to match w_mul scaling.
 * Depending on len mod p, constants differ for each method (p=2,3,5; TC3).
 */
#define XOR_CNT(len, alg) xor_cnt(len, alg)

uint64_t xor_cnt(uint64_t len, uint64_t alg) {
    uint64_t k;
    if (alg == KARAT2) { // 2-way Karatsuba
        k = len / 2;
        if (len % 2 == 0) return 100 * (7 * k);
        else return 100 * (7 * k + 3);
    }
    if (alg == KARAT3) { // 3-way Karatsuba
        k = len / 3;
        if (len % 3 == 0) return 100 * (20 * k);
        if (len % 3 == 1) return 100 * (20 * k + 8);
        else return 100 * (20 * k + 14);
    }
    if (alg == KARAT5) { // 5-way Karatsuba
        k = len / 5;
        if (len % 5 == 0) return 100 * (80 * k);
        else return UINT32_MAX;
    }
    if (alg == TC3) { // Toom-Cook 3-way
        k = len / 3;
        if (len % 3 == 0) return 100 * (38 * k + 18);
        if (len % 3 == 1) return 100 * (38 * k + 22);
        else return 100 * (38 * k + 31);             
    }
}

/*
 * mul_cnt(len, alg, w):
 *  - Computes the contribution to total cost from base multiplications induced by
 *    one split at (len, alg). It uses the already-built DP table w_min for child
 *    lengths (since w_min encodes mul*w + XOR at scale ×100).
 *  - We don't count raw muls directly here—rather, we use the structure of each
 *    algorithm (child lengths and multiplicities) to sum child w_min terms.
 */
uint64_t mul_cnt(uint64_t len, uint64_t alg, int w) {
    uint64_t k;
    if (alg == 0) { // 2-way Karatsuba
        k = len / 2;
        if (len % 2 == 0) return 3 * w_min[w][k];
        else return w_min[w][k] + 2 * w_min[w][k + 1];
    }
    if (alg == 1) { // 3-way Karatsuba
#if COMPARE_WITH_GF2XLIB == 1
        return (uint64_t)UINT_MAX * 1024; // artificially penalize when comparing with gf2x
#endif
        k = len / 3;
        if (len % 3 == 0) return 6 * w_min[w][k];
        if (len % 3 == 1) return 3 * w_min[w][k] + 3 * w_min[w][k + 1];
        else return w_min[w][k] + 5 * w_min[w][k + 1];
    }
    if (alg == 2) { // 5-way Karatsuba
#if COMPARE_WITH_GF2XLIB == 1
        return UINT_MAX; // artificially penalize when comparing with gf2x
#endif
        k = len / 5;
        if (len % 5 == 0) return 15 * w_min[w][k];
        else return UINT32_MAX;
    }
    if (alg == 3) { // Toom-Cook 3-way
        k = len / 3;
        if (len % 3 == 0) return 3 * w_min[w][k] + 2 * w_min[w][k + 2];
        if (len % 3 == 1) return w_min[w][k - 1] + 2 * w_min[w][k + 1] + 2 * w_min[w][k + 2];
        else return w_min[w][k] + 2 * w_min[w][k + 1] + 2 * w_min[w][k + 2];
        // else return UINT32_MAX;
    }
}
void store_subtree(uint64_t len, int depth, int mul_weight) {
    int i;
    /* Check if sub-length 'len' is already recorded for this weight (avoid duplicates). */
    for (i = 0; i < length_before_len[mul_weight]; i++) {
        if (length_before[mul_weight][i] == len) break;
    }
    /* Ignore duplicates and the trivial leaf len=1. */
    if ((i != length_before_len[mul_weight]) || (len == 1)) {
        return;
    }
    /* Record the newly observed sub-length and its chosen first algorithm. */
    length_before[mul_weight][length_before_len[mul_weight]] = len;
    firstalg_before[mul_weight][length_before_len[mul_weight]++] = first_alg[mul_weight][len];

    /* Recurse to children according to the chosen algorithm and remainder pattern. */
    if (first_alg[mul_weight][len] == 0) {
        store_subtree(len / 2, depth + 1, mul_weight);
        if (len % 2 != 0) {
            store_subtree(len / 2 + 1, depth + 1, mul_weight);
        }
    }
    else if (first_alg[mul_weight][len] == 1) {
        store_subtree(len / 3, depth + 1, mul_weight);
        if (len % 3 != 0) {
            store_subtree(len / 3 + 1, depth + 1, mul_weight);
        }
    }
    else if (first_alg[mul_weight][len] == 2) {
        store_subtree(len / 5, depth + 1, mul_weight);
    }
    else if (first_alg[mul_weight][len] == 3) {
        if (len % 3 == 0) {
            store_subtree(len / 3, depth + 1, mul_weight);
            store_subtree(len / 3 + 2, depth + 1, mul_weight);
        }
        if (len % 3 == 1) {
            store_subtree(len / 3 - 1, depth + 1, mul_weight);
            store_subtree(len / 3 + 1, depth + 1, mul_weight);
            store_subtree(len / 3 + 2, depth + 1, mul_weight);
        }
        if (len % 3 == 2) {
            store_subtree(len / 3, depth + 1, mul_weight);
            store_subtree(len / 3 + 1, depth + 1, mul_weight);
            store_subtree(len / 3 + 2, depth + 1, mul_weight);
        }
    }

    else if (first_alg[mul_weight][len] == 4) {
        /* Unused fallback branch kept as a guard. */
        if (len % 3 == 0) {
            store_subtree(len / 3, depth + 1, mul_weight);
            store_subtree(len / 3 + 1, depth + 1, mul_weight);
        }
        else if (len % 3 == 1) {
            store_subtree(len / 3, depth + 1, mul_weight);
            store_subtree(len / 3 + 1, depth + 1, mul_weight);
        }
        else if (len % 3 == 2) {
            store_subtree(len / 3 + 1, depth + 1, mul_weight);
        }
    }
}

//
// // 
// --- Accumulate internal-call statistics ------------------------------------
/*
 * accumulate_calls():
 *  - Starting at 'len' with multiplicity 'factor', follow first_alg[w][len] downwards
 *    and add to calls_by_len[L] how many subcalls of each sub-length L occur.
 *  - alg_nodes[alg] counts how many parent nodes use each algorithm (for stats).
 */
static void accumulate_calls(uint64_t len, uint64_t w, uint64_t factor,
    uint64_t* calls_by_len, uint64_t* alg_nodes)
{
    if (len <= 1) return;                 // base length => no internal calls
    uint64_t alg = first_alg[w][len];
    alg_nodes[alg] += factor;             // count parent nodes by algorithm

    if (alg == KARAT2) {
        uint64_t k = len / 2;
        if ((len % 2) == 0) {
            calls_by_len[k] += 3 * factor;
            accumulate_calls(k, w, 3 * factor, calls_by_len, alg_nodes);
        }
        else {
            calls_by_len[k] += 1 * factor;
            calls_by_len[k + 1] += 2 * factor;
            accumulate_calls(k, w, 1 * factor, calls_by_len, alg_nodes);
            accumulate_calls(k + 1, w, 2 * factor, calls_by_len, alg_nodes);
        }
    }
    else if (alg == KARAT3) {
        uint64_t k = len / 3;
        if ((len % 3) == 0) {
            calls_by_len[k] += 6 * factor;
            accumulate_calls(k, w, 6 * factor, calls_by_len, alg_nodes);
        }
        else if ((len % 3) == 1) {
            calls_by_len[k] += 3 * factor;
            calls_by_len[k + 1] += 3 * factor;
            accumulate_calls(k, w, 3 * factor, calls_by_len, alg_nodes);
            accumulate_calls(k + 1, w, 3 * factor, calls_by_len, alg_nodes);
        }
        else { // 3k+2
            calls_by_len[k] += 1 * factor;
            calls_by_len[k + 1] += 5 * factor;
            accumulate_calls(k, w, 1 * factor, calls_by_len, alg_nodes);
            accumulate_calls(k + 1, w, 5 * factor, calls_by_len, alg_nodes);
        }
    }
    else if (alg == KARAT5) {
        uint64_t k = len / 5; // len%5==0 assumed (otherwise not chosen)
        calls_by_len[k] += 15 * factor;
        accumulate_calls(k, w, 15 * factor, calls_by_len, alg_nodes);
    }
    else if (alg == TC3) {
        uint64_t k = len / 3;
        if ((len % 3) == 0) {             // 3k: children k×3 and (k+2)×2
            calls_by_len[k] += 3 * factor;
            calls_by_len[k + 2] += 2 * factor;
            accumulate_calls(k, w, 3 * factor, calls_by_len, alg_nodes);
            accumulate_calls(k + 2, w, 2 * factor, calls_by_len, alg_nodes);
        }
        else if ((len % 3) == 1) {        // 3k+1: (k-1)×1, (k+1)×2, (k+2)×2
            calls_by_len[k - 1] += 1 * factor;
            calls_by_len[k + 1] += 2 * factor;
            calls_by_len[k + 2] += 2 * factor;
            accumulate_calls(k - 1, w, 1 * factor, calls_by_len, alg_nodes);
            accumulate_calls(k + 1, w, 2 * factor, calls_by_len, alg_nodes);
            accumulate_calls(k + 2, w, 2 * factor, calls_by_len, alg_nodes);
        }
        else {                              // 3k+2: k×1, (k+1)×2, (k+2)×2
            calls_by_len[k] += 1 * factor;
            calls_by_len[k + 1] += 2 * factor;
            calls_by_len[k + 2] += 2 * factor;
            accumulate_calls(k, w, 1 * factor, calls_by_len, alg_nodes);
            accumulate_calls(k + 1, w, 2 * factor, calls_by_len, alg_nodes);
            accumulate_calls(k + 2, w, 2 * factor, calls_by_len, alg_nodes);
        }
    }
}

/*
 * print_call_stats():
 *  - Build histogram of subcalls per sub-length for the chosen tree, and
 *    print the total internal function calls.
 *  - Also collects (lens[], cnts[]) for lengths that actually appeared.
 *    (Not printed here; use externally if desired.)
 */
static void print_call_stats(uint64_t top_len, uint64_t w_index)
{
    // Per-length call histogram
    static uint64_t calls_by_len[POLY_LENGTH + 1];
    memset(calls_by_len, 0, sizeof(calls_by_len));

    // Parent-node counts by algorithm (for stats)
    uint64_t alg_nodes[5] = { 0, 0, 0, 0, 0 };

    accumulate_calls(top_len, w_index, 1, calls_by_len, alg_nodes);

    uint64_t total_calls = 0;
    for (size_t L = 1; L <= POLY_LENGTH; ++L) total_calls += calls_by_len[L];

    printf("Total internal function calls : %llu\n", (unsigned long long)total_calls);

    // Collect (len, count) pairs for lengths actually observed in this tree
    uint64_t lens[100] = { 0 }, cnts[100] = { 0 };
    int used = length_before_len[w_index];
    for (int j = 0; j < used; ++j) {
        uint64_t L = length_before[w_index][j];
        lens[j] = L;
        cnts[j] = calls_by_len[L];
    }
}


/* Print the tree textually (avoid duplicate prints via print_before[]) */
void print_subtree(uint64_t len, int depth, uint64_t mul_w, uint64_t* print_before, int* print_before_len) {
    int i;
    printf("%3ld ", len);
    for (i = 0; i < *print_before_len; i++) {
        if (print_before[i] == len) break;
    }
    if (i == *print_before_len && len != 1) {
        print_before[(*print_before_len)++] = len;
    }
    else {
        printf("\n");
        return;
    }

    /* Tag the chosen algorithm at this node */
    if (first_alg[mul_w][len] == KARAT2) printf("2-k\t");
    else if (first_alg[mul_w][len] == KARAT3) printf("3-k\t");
    else if (first_alg[mul_w][len] == KARAT5) printf("5-k\t");
    else if (first_alg[mul_w][len] == TC3) printf("tc3\t");
    else printf("err\t");

    /* Recurse into children depending on algorithm and remainder */
    if (first_alg[mul_w][len] == KARAT2) {
        // printf("alg0,depth=%d   ",*depth);
        print_subtree(len / 2, depth + 1, mul_w, print_before, print_before_len);
        if (len % 2 != 0) {
            for (i = 0; i < depth + 1; i++) printf("\t");
            print_subtree(len / 2 + 1, depth + 1, mul_w, print_before, print_before_len);
        }
    }
    else if (first_alg[mul_w][len] == KARAT3) {
        // printf("alg1,depth=%d   ", *depth);
        print_subtree(len / 3, depth + 1, mul_w, print_before, print_before_len);
        if (len % 3 != 0) {
            for (i = 0; i < depth + 1; i++) printf("\t");
            print_subtree(len / 3 + 1, depth + 1, mul_w, print_before, print_before_len);
        }
    }
    else if (first_alg[mul_w][len] == KARAT5) {
        // printf("alg2,depth=%d   ", *depth);
        if (len % 5 != 0) printf("\nError : 5karat - len = %ld is not a 5k\n", len);
        else print_subtree(len / 5, depth + 1, mul_w, print_before, print_before_len);
    }
    else if (first_alg[mul_w][len] == TC3) {
        // printf("alg3,depth=%d   ", *depth);
        if (len % 3 == 0) {
            print_subtree(len / 3, depth + 1, mul_w, print_before, print_before_len);
            for (i = 0; i < depth + 1; i++) printf("\t");
            print_subtree(len / 3 + 2, depth + 1, mul_w, print_before, print_before_len);
        }
        if (len % 3 == 1) {
            print_subtree(len / 3 - 1, depth + 1, mul_w, print_before, print_before_len);
            for (i = 0; i < depth + 1; i++) printf("\t");
            print_subtree(len / 3 + 1, depth + 1, mul_w, print_before, print_before_len);
            for (i = 0; i < depth + 1; i++) printf("\t");
            print_subtree(len / 3 + 2, depth + 1, mul_w, print_before, print_before_len);
        }
        if (len % 3 == 2) {
            print_subtree(len / 3, depth + 1, mul_w, print_before, print_before_len);
            for (i = 0; i < depth + 1; i++) printf("\t");
            print_subtree(len / 3 + 1, depth + 1, mul_w, print_before, print_before_len);
            for (i = 0; i < depth + 1; i++) printf("\t");
            print_subtree(len / 3 + 2, depth + 1, mul_w, print_before, print_before_len);
        }
    }
}

/* Print banner + tree (shows the w range where this structure is optimal) */
void print_tree(uint64_t LENGTH, uint64_t n) {
    int end = 0;
    int depth = 0;
    printf("************* (case %ld) w_mul = %.2f ~ ", n + 1, (float)diff_tree_weight[n] / 100);
    if (n != diff_tree_w_len - 1) {
        printf("%.2f", (float)(diff_tree_weight[n + 1] - 1) / 100);
    }
    else {
        printf("%.2f", (float)MAX_W / 100);
    }
    printf(" : algorithm tree for a polynomial of length %ld ***************\n\n", LENGTH);

    uint64_t print_before[100] = { 0 };
    int print_before_len = 0;
    print_subtree(LENGTH, depth, diff_tree_weight[n], print_before, &print_before_len);
}

int main()
{
    size_t i, j, w;
    uint64_t tmp;
    uint16_t p, r;
    uint64_t w_2karat;  // cost if computed via 2-way Karatsuba at this (w,len)
    uint64_t w_3karat;
    uint64_t w_5karat;
    uint64_t w_tc3;
    uint64_t muls[POLY_LENGTH + 1];
    uint64_t muls_final[MAX_W];
    uint64_t XORs[POLY_LENGTH + 1];
    uint64_t XORs_final[MAX_W];

    /*
     * Base cases (len=1..4) to seed DP.
     * If COMPARE_WITH_GF2XLIB==1, some paths are forced for comparison experiments.
     * Note: weights start at w=100 (i.e., 1.00).
     */
    for (w = 100; w < MAX_W; w++) {
        w_min[w][1] = w;
        w_min[w][2] = 3 * w_min[w][1] + XOR_CNT(2, KARAT2);
#if COMPARE_WITH_GF2XLIB == 1
        w_min[w][3] = w_min[w][1] + 2 * w_min[w][2] + XOR_CNT(3, KARAT2);
#else
        w_min[w][3] = 6 * w_min[w][1] + XOR_CNT(3, KARAT3);
#endif
        w_min[w][4] = 3 * w_min[w][2] + XOR_CNT(4, KARAT2);

        first_alg[w][1] = -1;
        first_alg[w][2] = KARAT2;
#if COMPARE_WITH_GF2XLIB == 1
        first_alg[w][3] = KARAT2; // forced for gf2x comparison
#else
        first_alg[w][3] = KARAT3;
#endif
        first_alg[w][4] = KARAT2;
        muls[1] = 1;
        muls[2] = 3;
#if COMPARE_WITH_GF2XLIB == 1
        muls[3] = 7; // forced for gf2x comparison
#else
        muls[3] = 6;
#endif
        muls[4] = 9;
        XORs[1] = 0;
        XORs[2] = xor_cnt(2, KARAT2);
#if COMPARE_WITH_GF2XLIB == 1
        XORs[3] = 2 * XORs[2] + xor_cnt(3, KARAT2); // forced for gf2x comparison
#else
        XORs[3] = xor_cnt(3, KARAT3);
#endif
        XORs[4] = 3 * XORs[2] + xor_cnt(4, KARAT2);
    }

    /*
     * Main DP loop for len=5..POLY_LENGTH:
     *  - For each len, evaluate candidates KARAT2/3/5/TC3 using mul_cnt()+XOR_CNT()
     *  - Take the minimum as w_min[w][len], record first_alg[w][len], and
     *    update the derived cumulative counts (muls[len], XORs[len]) accordingly.
     */
    for (w = 100; w < MAX_W; w++) {
        for (i = 5; i <= POLY_LENGTH; i++) {
            w_min[w][i] = UINT64_MAX;

            w_2karat = mul_cnt(i, KARAT2, w) + XOR_CNT(i, KARAT2);
            w_3karat = mul_cnt(i, KARAT3, w) + XOR_CNT(i, KARAT3);
            w_5karat = mul_cnt(i, KARAT5, w) + XOR_CNT(i, KARAT5);
            w_tc3 = mul_cnt(i, TC3, w) + XOR_CNT(i, TC3);

            w_min[w][i] = min(w_2karat, w_3karat);
            w_min[w][i] = min(w_min[w][i], w_5karat);
            w_min[w][i] = min(w_min[w][i], w_tc3);
            if (w_5karat == w_min[w][i]) {
                first_alg[w][i] = KARAT5;
                /* 5-way Karatsuba: 15 child multiplications of equal size (i/5) */
                muls[i] = 15 * muls[i / 5];
                XORs[i] = 15 * XORs[i / 5] + xor_cnt(i, KARAT5);
            }
            else if (w_3karat == w_min[w][i]) {
                first_alg[w][i] = KARAT3;
                /* 3-way Karatsuba: case splits by i % 3 */
                if (i % 3 == 0) {
                    muls[i] = 6 * muls[i / 3];
                    XORs[i] = 6 * XORs[i / 3] + xor_cnt(i, KARAT3);
                }
                else if (i % 3 == 1) {
                    muls[i] = 3 * muls[i / 3] + 3 * muls[i / 3 + 1];
                    XORs[i] = 3 * XORs[i / 3] + 3 * XORs[i / 3 + 1] + xor_cnt(i, KARAT3);
                }
                else {
                    muls[i] = muls[i / 3] + 5 * muls[i / 3 + 1];
                    XORs[i] = XORs[i / 3] + 5 * XORs[i / 3 + 1] + xor_cnt(i, KARAT3);
                }
            }
            else if (w_2karat == w_min[w][i]) {
                first_alg[w][i] = KARAT2;
                /* 2-way Karatsuba: even vs odd split (k,k) or (k,k+1,k+1) */
                if (i % 2 == 0) {
                    muls[i] = 3 * muls[i / 2];
                    XORs[i] = 3 * XORs[i / 2] + xor_cnt(i, KARAT2);
                }
                else {
                    muls[i] = muls[i / 2] + 2 * muls[i / 2 + 1];
                    XORs[i] = XORs[i / 2] + 2 * XORs[i / 2 + 1] + xor_cnt(i, KARAT2);
                }
            }
            else if (w_tc3 == w_min[w][i]) {
                first_alg[w][i] = TC3;
                /* Toom-Cook 3-way: three cases by i % 3 (uses k, k+1, k+2, and occasionally k-1) */
                if (i % 3 == 0) {
                    muls[i] = 3 * muls[i / 3] + 2 * muls[i / 3 + 2];
                    XORs[i] = 3 * XORs[i / 3] + 2 * XORs[i / 3 + 2] + xor_cnt(i, TC3);
                }
                else if (i % 3 == 1) {
                    muls[i] = muls[i / 3 - 1] + 2 * muls[i / 3 + 1] + 2 * muls[i / 3 + 2];
                    XORs[i] = XORs[i / 3 - 1] + 2 * XORs[i / 3 + 1] + 2 * XORs[i / 3 + 2] + xor_cnt(i, TC3);
                }
                else {
                    muls[i] = muls[i / 3] + 2 * muls[i / 3 + 1] + 2 * muls[i / 3 + 2];
                    XORs[i] = XORs[i / 3] + 2 * XORs[i / 3 + 1] + 2 * XORs[i / 3 + 2] + xor_cnt(i, TC3);
                }
            }
            else printf("err1");
        }
        /* Store top-level counts at this weight. */
        muls_final[w] = muls[POLY_LENGTH];
        XORs_final[w] = XORs[POLY_LENGTH];
    }

    /*
     * Collect tree structure per weight:
     *  - For each w, enumerate unique sub-lengths reachable from top via first_alg.
     *  - Record breakpoints in diff_tree_weight when the set of sub-lengths changes.
     */
    for (w = 100; w < MAX_W; w++) {
        store_subtree(POLY_LENGTH, 0, w);
    }
    diff_tree_weight[diff_tree_w_len++] = 100;
    tmp = 100;
    for (i = 100; i < MAX_W; i++) {
        if (length_before_len[i] != length_before_len[tmp] || !arr_compare(length_before[i], length_before[tmp], length_before_len[i])) {
            diff_tree_weight[diff_tree_w_len++] = i;
            tmp = i;
        }
    }

    /*
     * Output per structure-range:
     *  - Print the tree banner + textual tree.
     *  - Print total base multiplications (muls_final), total XOR count,
     *    and total internal-call statistics.
     *  - Check the consistency: w_min[top] == muls_final*w + XORs_final
     */
    for (i = 0; i < diff_tree_w_len; i++) {
        print_tree(POLY_LENGTH, i);

        //sort_print(length_before[diff_tree_weight[i]], firstalg_before[diff_tree_weight[i]], length_before_len[diff_tree_weight[i]]);

        printf("\n\nTotal basemuls  = %ld\n", muls_final[diff_tree_weight[i]]);

        printf("Total XOR count = %ld\n\n", XORs_final[diff_tree_weight[i]]);
        print_call_stats(POLY_LENGTH, diff_tree_weight[i]);
#if !ONLY_MUL
        if (w_min[diff_tree_weight[i]][POLY_LENGTH] == muls_final[diff_tree_weight[i]] * diff_tree_weight[i] + XORs_final[diff_tree_weight[i]]) {
            printf("Total weight sum = %ld * w_mul + %ld\n\n\n", muls_final[diff_tree_weight[i]], XORs_final[diff_tree_weight[i]]);
        }
        else printf("Total weight sum (%ld) != total unit multiplications * weight + total XOR count (%ld)\n\n\n", w_min[diff_tree_weight[i]][POLY_LENGTH], muls_final[diff_tree_weight[i]] * diff_tree_weight[i] + XORs_final[diff_tree_weight[i]]);
#endif
    }
    return 0;
}
