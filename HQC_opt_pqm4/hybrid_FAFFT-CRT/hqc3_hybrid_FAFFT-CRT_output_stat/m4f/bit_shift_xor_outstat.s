.syntax unified
    .cpu cortex-m4
    .thumb

@ =========================================================================
@ HQC-3 output-stationary sparse-dense multiplication.
@ Computes R := R + t(x) * P_{d+1}(x), where t(x) is the dense residual
@ polynomial and P_{d+1}(x) is the sparse fixed CRT modulus.
@ Implements Algorithm 4 of the paper (Section 3.2.2).
@
@ Arguments:
@   res       : result buffer R (accumulated in place)
@   t         : dense polynomial t(x), 193 words
@   voff      : per-block offsets into the precomputed shift table Tsh
@   cnt       : number of contributing terms per result block
@   nblocks   : number of 5-word result blocks
@   tail_voff : offsets into Tsh for the 1-word tail
@   tail_cnt  : number of contributing terms for the tail
@
@ Method:
@   1. Precompute 31 bit-shifted copies of t(x) into Tsh[0..31] via an
@      ADCS chain (single-cycle add-with-carry emulates multi-word x << 1).
@   2. Per-block accumulation, batch size alpha = 5, XOR-ing the
@      contributing slices of Tsh into the current 5-word result block.
@   3. 1-word tail loop handles the remaining word at the end of R.
@ =========================================================================

    .section .bss
    .balign 4
Tsh_buf:
    .space 25856                       @ 32 rows × 202 words × 4 bytes

    .section .text
    .thumb_func
    .global bit_shift_xor_193_output_asm

bit_shift_xor_193_output_asm:
    PUSH    {R4-R11, LR}               @ callee-saved
    PUSH    {R0, R2, R3}               @ save res / voff_ptr / cnt_ptr (reused during precompute)

@ ========== PRECOMPUTE ==========

    @ Step 1: copy t (R1, 193 words) into row 0 data area = Tsh_buf + 16
    LDR     R5, =Tsh_buf
    ADD     R5, R5, #16
    MOV     R6, #38                    @ 38 * 5 = 190 words
Copy_Loop:
    LDMIA   R1!, {R7-R11}
    STMIA   R5!, {R7-R11}
    SUBS    R6, R6, #1
    BNE     Copy_Loop
    LDMIA   R1!, {R7-R9}               @ 3 tail words -> 193 total
    STMIA   R5!, {R7-R9}

    @ Step 2: rows 1..31 = previous row << 1, via ADCS chain (N=11 in-place).
    @ 193 = 17 * 11 + 6  ->  17 full batches + 6-word tail.
    LDR     R5, =Tsh_buf
    ADD     R5, R5, #16                @ src = row 0 data start
    ADD     R6, R5, #808               @ dst = row 1 data start (ROW_STRIDE*4)
    MOV     R4, #31                    @ row counter (saved across inner)

Row_Loop:
    PUSH    {R4}                       @ stash row counter

    @ Clear APSR.C for this row's first ADCS (no incoming carry).
    @ LSLS by 1 sets C = bit 31 of source; with source = 0, C = 0.
    MOV     R0, #0
    LSLS    R0, R0, #1

    ADD     R4, R5, #748               @ inner end = src + 17*44 (17 batches of 11 words)

Inner_Shift:
    LDMIA   R5!, {R0-R3, R7-R12, LR}   @ 11 src words (in-place targets)
    ADCS    R0,  R0,  R0               @ R0 = R0+R0+C  ==  (R0<<1) | C_in,   C_out = old R0[31]
    ADCS    R1,  R1,  R1               @ carry chains automatically through APSR.C
    ADCS    R2,  R2,  R2
    ADCS    R3,  R3,  R3
    ADCS    R7,  R7,  R7
    ADCS    R8,  R8,  R8
    ADCS    R9,  R9,  R9
    ADCS    R10, R10, R10
    ADCS    R11, R11, R11
    ADCS    R12, R12, R12
    ADCS    LR,  LR,  LR               @ C now holds outgoing carry for next batch
    STMIA   R6!, {R0-R3, R7-R12, LR}
    TEQ     R5, R4                     @ Z = (R5 == R4); C PRESERVED
    BNE     Inner_Shift

    @ Tail: 6 words remaining (193 - 17*11 = 6). Same ADCS chain, smaller batch.
    LDMIA   R5!, {R0-R3, R7, R8}
    ADCS    R0, R0, R0
    ADCS    R1, R1, R1
    ADCS    R2, R2, R2
    ADCS    R3, R3, R3
    ADCS    R7, R7, R7
    ADCS    R8, R8, R8
    STMIA   R6!, {R0-R3, R7, R8}

    @ Advance to next row data start (skip drop word + guards = (202-193)*4 = 36 B)
    ADD     R5, R5, #36
    ADD     R6, R6, #36
    POP     {R4}
    SUBS    R4, R4, #1
    BNE     Row_Loop

    POP     {R0, R2, R3}               @ restore res / voff / cnt

@ ========== MAIN LOOP SETUP ==========

    LDR     R12, [SP, #36]             @ nblocks
    ADD     R12, R12, R12, LSL #2      @ * 5
    ADD     R12, R0,  R12, LSL #2      @ end_res = res + nblocks*20
    PUSH    {R12}                      @ stash end_res

    LDR     R1, =Tsh_buf          @ R1 = Tsh base

@ ========== Main loop (5-word output blocks) ==========
Block_Loop:
    LDMIA   R0, {R4-R8}
    LDR     R12, [R3], #4
    CMP     R12, #0
    BEQ     Store_Block
    PUSH    {R0, R3}
Term_Loop:
    LDR     LR,  [R2], #4
    ADD     LR,  R1, LR, LSL #2
    LDMIA   LR,  {R0, R3, R9, R10, R11}
    EOR     R4,  R4, R0
    EOR     R5,  R5, R3
    EOR     R6,  R6, R9
    EOR     R7,  R7, R10
    EOR     R8,  R8, R11
    SUBS    R12, R12, #1
    BNE     Term_Loop
    POP     {R0, R3}
Store_Block:
    STMIA   R0!, {R4-R8}
    ADD     R1,  R1,  #20
    LDR     R12, [SP]
    CMP     R0,  R12
    BLO     Block_Loop

@ ========== Tail (1 leftover word) ==========
    LDR     R12, [SP, #48]             @ tail_cnt
    CMP     R12, #0
    BEQ     Done
    LDR     R9,  [SP, #44]             @ tail_voff ptr
    LDR     R4,  [R0]                  @ a = res[tail_word]
Tail_Loop:
    LDR     LR,  [R9], #4
    ADD     LR,  R1, LR, LSL #2
    LDR     R10, [LR]
    EOR     R4,  R4, R10
    SUBS    R12, R12, #1
    BNE     Tail_Loop
    STR     R4,  [R0]

Done:
    ADD     SP, SP, #4
    POP     {R4-R11, PC}

    .ltorg
