.syntax unified
    .cpu cortex-m4
    .thumb
    .global bit_shift_xor_81_asm

@ -------------------------------------------------------------------------
@ void bit_shift_xor_81_asm(uint32_t* res, const uint32_t* A, 
@                           const uint32_t* idx, const uint32_t* cnt)
@ -------------------------------------------------------------------------
@ R0: res, R1: A(t), R2: LoopCnt, R3: Cnt_Ptr
@ Data: v1~v5 (R5~R9), Carry: v0 (R11)
@ -------------------------------------------------------------------------

bit_shift_xor_81_asm:
    PUSH    {R4-R11, LR}        @ Context Save

    @ 1. Loop Limit Setup
    @ Total 81 words. Process 5 words per block.
    @ Limit = 81 - 1 (tail) = 80 words.
    @ 80 words * 4 bytes = 320 bytes.
    MOVW    R12, #320           
    ADD     LR, R1, R12
    MOV     R11, #0

Outer_Loop_Start:
    CMP     R1, LR
    BHS     Outer_Loop_End

    @ [Load 5 Words] v1~v5 -> R5~R9
    @ R5:v1, R6:v2, R7:v3, R8:v4, R9:v5
    LDMIA   R1!, {R5-R9}       
    
    PUSH    {R1, R2, R3, LR}    @ Context Save (Preserve pointers)
    MOV     R4, R2              @ R4 = Idx Pointer Copy

    @ =========================================================
    @ Case s == 0
    @ =========================================================
    LDR     R2, [R3], #4        
    CMP     R2, #0
    BEQ     Shift_Loop_Setup

    @ R3 (Cnt Ptr) is not needed inside the XOR loop.
    @ R11 (Carry v0) is not needed inside the XOR loop (s=0).
    @ Push them to stack to FREE UP registers for LDMIA.
    PUSH    {R3, R11} 

Loop_S0:
    LDR     LR, [R4], #4        @ Load Idx
    ADD     LR, R0, LR, LSL #2  @ Address

    @ 5-Word LDMIA
    @ Available Regs: R1, R3, R10, R11, R12 (Total 5)
    LDMIA   LR, {R1, R3, R10, R11, R12}    
    EOR     R1,  R1,  R5        @ ^ v1
    EOR     R3,  R3,  R6        @ ^ v2
    EOR     R10, R10, R7        @ ^ v3
    EOR     R11, R11, R8        @ ^ v4
    EOR     R12, R12, R9        @ ^ v5
    STMIA   LR!, {R1, R3, R10, R11, R12}

    SUBS    R2, R2, #1
    BNE     Loop_S0

    POP     {R3, R11}           @ Restore

    @ =========================================================
    @ Shift Loop s = 1..31
    @ =========================================================
Shift_Loop_Setup:
    MOV     R1, #1              @ s = 1
    PUSH    {R1}                @ Save s

Loop_Shift_Start:
    @ v0(R11)->v1(R5)->...->v5(R9)
    LSL     R9, R9, #1
    ORR     R9, R9, R8, LSR #31
    LSL     R8, R8, #1
    ORR     R8, R8, R7, LSR #31
    LSL     R7, R7, #1
    ORR     R7, R7, R6, LSR #31
    LSL     R6, R6, #1
    ORR     R6, R6, R5, LSR #31
    LSL     R5, R5, #1
    ORR     R5, R5, R11, LSR #31    
    LSL     R11, R11, #1        

    @ Inner Loop Setup
    LDR     R2, [R3], #4        @ Cnt[s]
    CMP     R2, #0
    BEQ     Next_Shift_Step 

    @ Free up R3 and R11
    PUSH    {R3, R11}

Loop_S_Inner:
    LDR     LR, [R4], #4
    ADD     LR, R0, LR, LSL #2

    @ 5-Word Burst
    LDMIA   LR, {R1, R3, R10, R11, R12}    
    EOR     R1,  R1,  R5
    EOR     R3,  R3,  R6
    EOR     R10, R10, R7
    EOR     R11, R11, R8
    EOR     R12, R12, R9
    STMIA   LR!, {R1, R3, R10, R11, R12}

    SUBS    R2, R2, #1
    BNE     Loop_S_Inner

    POP     {R3, R11}           @ Restore

Next_Shift_Step:
    POP     {R1}                @ Restore s
    ADD     R1, R1, #1
    CMP     R1, #32
    PUSH    {R1}                @ Save s
    BLT     Loop_Shift_Start

    POP     {R1}                @ Clean s
    POP     {R1, R2, R3, LR}    @ Restore
    
    @ Reload v0 for Next Block 
    LDR     R11, [R1, #-4]      

    ADD     R0, R0, #20         @ res += 5 words
    B       Outer_Loop_Start

    @ ---------------------------------------------------------
    @ 3. Tail Processing (Only 1 Word left)
    @ ---------------------------------------------------------
Outer_Loop_End:
    @ R11 has Carry (t[79])
    LDMIA   R1, {R5}            @ v1=t[80]
    
    MOV     R4, R2              @ Reset Idx ptr
    LDR     R2, [R3], #4        @ Cnt[0]
    
    CMP     R2, #0
    BEQ     Tail_Shift_Setup

Tail_S0:
    LDR     LR, [R4], #4
    ADD     LR, R0, LR, LSL #2

    LDR     R12, [LR]           
    EOR     R12, R12, R5
    STR     R12, [LR]

    SUBS    R2, R2, #1
    BNE     Tail_S0

Tail_Shift_Setup:
    MOV     R1, #1

Tail_Shift_Start:
    @ Shift Chain
    LSL     R5, R5, #1
    ORR     R5, R5, R11, LSR #31
    LSL     R11, R11, #1

    LDR     R2, [R3], #4
    CMP     R2, #0
    BEQ     Tail_Next_Shift

Tail_S_Inner:
    LDR     LR, [R4], #4
    ADD     LR, R0, LR, LSL #2

    LDR     R12, [LR]
    EOR     R12, R12, R5
    STR     R12, [LR]

    SUBS    R2, R2, #1
    BNE     Tail_S_Inner

Tail_Next_Shift:
    ADD     R1, R1, #1
    CMP     R1, #32
    BLT     Tail_Shift_Start

    POP     {R4-R11, PC}
    