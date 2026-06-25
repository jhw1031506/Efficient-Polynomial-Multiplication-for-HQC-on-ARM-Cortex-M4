.syntax unified
.cpu cortex-m4
.fpu fpv4-sp-d16
.global decode_from_gft
.type decode_from_gft, %function
.align 2
decode_from_gft:
stmdb   sp!, {r4-r11, lr}
sub     sp, sp, #4
vpush   {d8-d15}
vldr.32 s0, [r1, #0]
vldr.32 s1, [r1, #4]
vldr.32 s2, [r1, #8]
vldr.32 s3, [r1, #12]
vldr.32 s4, [r1, #16]
vldr.32 s5, [r1, #20]
vldr.32 s6, [r1, #24]
vldr.32 s7, [r1, #28]
vldr.32 s8, [r1, #32]
vldr.32 s9, [r1, #36]
vldr.32 s10, [r1, #40]
vldr.32 s11, [r1, #44]
vldr.32 s12, [r1, #48]
vldr.32 s13, [r1, #52]
vldr.32 s14, [r1, #56]
vldr.32 s15, [r1, #60]
vldr.32 s16, [r1, #64]
vldr.32 s17, [r1, #68]
vldr.32 s18, [r1, #72]
vldr.32 s19, [r1, #76]
vldr.32 s20, [r1, #80]
vldr.32 s21, [r1, #84]
vldr.32 s22, [r1, #88]
vldr.32 s23, [r1, #92]
vldr.32 s24, [r1, #96]
vldr.32 s25, [r1, #100]
vldr.32 s26, [r1, #104]
vldr.32 s27, [r1, #108]
vldr.32 s28, [r1, #112]
vldr.32 s29, [r1, #116]
vldr.32 s30, [r1, #120]
vldr.32 s31, [r1, #124]
push {r0, r1}
vmov r0, s23    @ load x[23] from s23
vmov r1, s31    @ load x[31] from s31
eor r0, r0, r1    @ x[23] = x[23] ^ x[31]
vmov r2, s22    @ load x[22] from s22
vmov r3, s25    @ load x[25] from s25
eor r2, r2, r3    @ x[22] = x[22] ^ x[25]
vmov r4, s7    @ load x[7] from s7
vmov r5, s21    @ load x[21] from s21
eor r4, r4, r5    @ x[7] = x[7] ^ x[21]
vmov r6, s18    @ load x[18] from s18
eor r1, r1, r6    @ x[31] = x[31] ^ x[18]
eor r2, r2, r1    @ x[22] = x[22] ^ x[31]
eor r4, r4, r2    @ x[7] = x[7] ^ x[22]
vmov r7, s17    @ load x[17] from s17
vmov r8, s28    @ load x[28] from s28
eor r7, r7, r8    @ x[17] = x[17] ^ x[28]
vmov r9, s1    @ load x[1] from s1
vmov r10, s4    @ load x[4] from s4
eor r9, r9, r10    @ x[1] = x[1] ^ x[4]
vmov r11, s3    @ load x[3] from s3
vmov r12, s15    @ load x[15] from s15
eor r11, r11, r12    @ x[3] = x[3] ^ x[15]
vmov r14, s13    @ load x[13] from s13
eor r9, r9, r14    @ x[1] = x[1] ^ x[13]
vmov s22, r2    @ spill x[22] from r2
vmov r2, s16    @ load x[16] from s16
eor r2, r2, r0    @ x[16] = x[16] ^ x[23]
vmov s25, r3    @ spill x[25] from r3
vmov r3, s5    @ load x[5] from s5
eor r14, r14, r3    @ x[13] = x[13] ^ x[5]
vmov s28, r8    @ spill x[28] from r8
vmov r8, s2    @ load x[2] from s2
eor r3, r3, r8    @ x[5] = x[5] ^ x[2]
vmov s16, r2    @ spill x[16] from r2
vmov r2, s19    @ load x[19] from s19
vmov s3, r11    @ spill x[3] from r11
vmov r11, s24    @ load x[24] from s24
eor r2, r2, r11    @ x[19] = x[19] ^ x[24]
vmov s23, r0    @ spill x[23] from r0
vmov r0, s6    @ load x[6] from s6
vmov s4, r10    @ spill x[4] from r10
vmov r10, s9    @ load x[9] from s9
eor r0, r0, r10    @ x[6] = x[6] ^ x[9]
eor r14, r14, r11    @ x[13] = x[13] ^ x[24]
eor r3, r3, r7    @ x[5] = x[5] ^ x[17]
vmov s13, r14    @ spill x[13] from r14
vmov r14, s11    @ load x[11] from s11
vmov s17, r7    @ spill x[17] from r7
vmov r7, s8    @ load x[8] from s8
eor r14, r14, r7    @ x[11] = x[11] ^ x[8]
vmov s24, r11    @ spill x[24] from r11
vmov r11, s29    @ load x[29] from s29
eor r7, r7, r11    @ x[8] = x[8] ^ x[29]
eor r12, r12, r6    @ x[15] = x[15] ^ x[18]
eor r7, r7, r9    @ x[8] = x[8] ^ x[1]
vmov s18, r6    @ spill x[18] from r6
vmov r6, s14    @ load x[14] from s14
eor r9, r9, r6    @ x[1] = x[1] ^ x[14]
eor r6, r6, r1    @ x[14] = x[14] ^ x[31]
vmov s1, r9    @ spill x[1] from r9
vmov r9, s0    @ load x[0] from s0
vmov s14, r6    @ spill x[14] from r6
vmov r6, s10    @ load x[10] from s10
eor r9, r9, r6    @ x[0] = x[0] ^ x[10]
eor r5, r5, r1    @ x[21] = x[21] ^ x[31]
eor r12, r12, r10    @ x[15] = x[15] ^ x[9]
eor r11, r11, r2    @ x[29] = x[29] ^ x[19]
vmov s29, r11    @ spill x[29] from r11
vmov r11, s12    @ load x[12] from s12
eor r7, r7, r11    @ x[8] = x[8] ^ x[12]
eor r3, r3, r4    @ x[5] = x[5] ^ x[7]
eor r11, r11, r4    @ x[12] = x[12] ^ x[7]
eor r9, r9, r11    @ x[0] = x[0] ^ x[12]
eor r4, r4, r0    @ x[7] = x[7] ^ x[6]
eor r12, r12, r14    @ x[15] = x[15] ^ x[11]
eor r14, r14, r8    @ x[11] = x[11] ^ x[2]
vmov s11, r14    @ spill x[11] from r14
vmov r14, s4    @ load x[4] from s4
eor r14, r14, r10    @ x[4] = x[4] ^ x[9]
vmov s8, r7    @ spill x[8] from r7
vmov r7, s23    @ load x[23] from s23
eor r5, r5, r7    @ x[21] = x[21] ^ x[23]
eor r8, r8, r11    @ x[2] = x[2] ^ x[12]
vmov s2, r8    @ spill x[2] from r8
vmov r8, s26    @ load x[26] from s26
eor r7, r7, r8    @ x[23] = x[23] ^ x[26]
vmov s15, r12    @ spill x[15] from r12
vmov r12, s14    @ load x[14] from s14
eor r11, r11, r12    @ x[12] = x[12] ^ x[14]
eor r12, r12, r6    @ x[14] = x[14] ^ x[10]
eor r11, r11, r3    @ x[12] = x[12] ^ x[5]
eor r0, r0, r6    @ x[6] = x[6] ^ x[10]
vmov s6, r0    @ spill x[6] from r0
vmov r0, s1    @ load x[1] from s1
vmov s14, r12    @ spill x[14] from r12
vmov r12, s3    @ load x[3] from s3
eor r0, r0, r12    @ x[1] = x[1] ^ x[3]
eor r6, r6, r0    @ x[10] = x[10] ^ x[1]
vmov s21, r5    @ spill x[21] from r5
vmov r5, s16    @ load x[16] from s16
eor r6, r6, r5    @ x[10] = x[10] ^ x[16]
eor r0, r0, r4    @ x[1] = x[1] ^ x[7]
vmov s10, r6    @ spill x[10] from r6
vmov r6, s27    @ load x[27] from s27
eor r4, r4, r6    @ x[7] = x[7] ^ x[27]
eor r10, r10, r3    @ x[9] = x[9] ^ x[5]
eor r6, r6, r2    @ x[27] = x[27] ^ x[19]
eor r7, r7, r2    @ x[23] = x[23] ^ x[19]
vmov s9, r10    @ spill x[9] from r10
vmov r10, s24    @ load x[24] from s24
eor r2, r2, r10    @ x[19] = x[19] ^ x[24]
eor r0, r0, r2    @ x[1] = x[1] ^ x[19]
vmov s1, r0    @ spill x[1] from r0
vmov r0, s18    @ load x[18] from s18
eor r2, r2, r0    @ x[19] = x[19] ^ x[18]
eor r3, r3, r5    @ x[5] = x[5] ^ x[16]
eor r5, r5, r0    @ x[16] = x[16] ^ x[18]
vmov s19, r2    @ spill x[19] from r2
vmov r2, s28    @ load x[28] from s28
eor r5, r5, r2    @ x[16] = x[16] ^ x[28]
eor r0, r0, r8    @ x[18] = x[18] ^ x[26]
eor r9, r9, r2    @ x[0] = x[0] ^ x[28]
eor r12, r12, r8    @ x[3] = x[3] ^ x[26]
eor r1, r1, r8    @ x[31] = x[31] ^ x[26]
vmov s31, r1    @ spill x[31] from r1
vmov r1, s25    @ load x[25] from s25
eor r14, r14, r1    @ x[4] = x[4] ^ x[25]
vmov s24, r10    @ spill x[24] from r10
vmov r10, s21    @ load x[21] from s21
eor r11, r11, r10    @ x[12] = x[12] ^ x[21]
vmov s21, r10    @ spill x[21] from r10
vmov r10, s15    @ load x[15] from s15
eor r10, r10, r6    @ x[15] = x[15] ^ x[27]
vmov s27, r6    @ spill x[27] from r6
vmov r6, s8    @ load x[8] from s8
eor r11, r11, r6    @ x[12] = x[12] ^ x[8]
vmov s15, r10    @ spill x[15] from r10
vmov r10, s29    @ load x[29] from s29
eor r5, r5, r10    @ x[16] = x[16] ^ x[29]
vmov s16, r5    @ spill x[16] from r5
vmov r5, s22    @ load x[22] from s22
eor r8, r8, r5    @ x[26] = x[26] ^ x[22]
vmov s29, r10    @ spill x[29] from r10
vmov r10, s17    @ load x[17] from s17
eor r4, r4, r10    @ x[7] = x[7] ^ x[17]
vmov s22, r5    @ spill x[22] from r5
vmov r5, s2    @ load x[2] from s2
eor r5, r5, r12    @ x[2] = x[2] ^ x[3]
eor r12, r12, r2    @ x[3] = x[3] ^ x[28]
vmov s2, r5    @ spill x[2] from r5
vmov r5, s20    @ load x[20] from s20
eor r8, r8, r5    @ x[26] = x[26] ^ x[20]
eor r2, r2, r7    @ x[28] = x[28] ^ x[23]
vmov s26, r8    @ spill x[26] from r8
vmov r8, s13    @ load x[13] from s13
eor r9, r9, r8    @ x[0] = x[0] ^ x[13]
eor r4, r4, r12    @ x[7] = x[7] ^ x[3]
eor r3, r3, r1    @ x[5] = x[5] ^ x[25]
eor r4, r4, r8    @ x[7] = x[7] ^ x[13]
eor r8, r8, r14    @ x[13] = x[13] ^ x[4]
vmov s0, r9    @ spill x[0] from r9
vmov r9, s10    @ load x[10] from s10
eor r9, r9, r11    @ x[10] = x[10] ^ x[12]
eor r14, r14, r0    @ x[4] = x[4] ^ x[18]
eor r14, r14, r12    @ x[4] = x[4] ^ x[3]
eor r14, r14, r6    @ x[4] = x[4] ^ x[8]
vmov s7, r4    @ spill x[7] from r4
vmov r4, s14    @ load x[14] from s14
eor r12, r12, r4    @ x[3] = x[3] ^ x[14]
vmov s23, r7    @ spill x[23] from r7
vmov r7, s15    @ load x[15] from s15
eor r4, r4, r7    @ x[14] = x[14] ^ x[15]
eor r7, r7, r6    @ x[15] = x[15] ^ x[8]
eor r6, r6, r10    @ x[8] = x[8] ^ x[17]
vmov s25, r1    @ spill x[25] from r1
vmov r1, s9    @ load x[9] from s9
vmov s13, r8    @ spill x[13] from r8
vmov r8, s30    @ load x[30] from s30
eor r1, r1, r8    @ x[9] = x[9] ^ x[30]
eor r10, r10, r0    @ x[17] = x[17] ^ x[18]
vmov s17, r10    @ spill x[17] from r10
vmov r10, s6    @ load x[6] from s6
eor r6, r6, r10    @ x[8] = x[8] ^ x[6]
vmov s15, r7    @ spill x[15] from r7
vmov r7, s24    @ load x[24] from s24
eor r0, r0, r7    @ x[18] = x[18] ^ x[24]
vmov s9, r1    @ spill x[9] from r1
vmov r1, s11    @ load x[11] from s11
eor r10, r10, r1    @ x[6] = x[6] ^ x[11]
vmov s12, r11    @ spill x[12] from r11
vmov r11, s22    @ load x[22] from s22
eor r7, r7, r11    @ x[24] = x[24] ^ x[22]
eor r0, r0, r2    @ x[18] = x[18] ^ x[28]
eor r11, r11, r8    @ x[22] = x[22] ^ x[30]
eor r6, r6, r2    @ x[8] = x[8] ^ x[28]
eor r2, r2, r8    @ x[28] = x[28] ^ x[30]
eor r14, r14, r8    @ x[4] = x[4] ^ x[30]
eor r3, r3, r4    @ x[5] = x[5] ^ x[14]
eor r4, r4, r1    @ x[14] = x[14] ^ x[11]
eor r1, r1, r8    @ x[11] = x[11] ^ x[30]
vmov s5, r3    @ spill x[5] from r3
vmov r3, s31    @ load x[31] from s31
eor r8, r8, r3    @ x[30] = x[30] ^ x[31]
eor r9, r9, r3    @ x[10] = x[10] ^ x[31]
vmov s18, r0    @ spill x[18] from r0
vmov r0, s29    @ load x[29] from s29
eor r3, r3, r0    @ x[31] = x[31] ^ x[29]
eor r10, r10, r12    @ x[6] = x[6] ^ x[3]
eor r12, r12, r5    @ x[3] = x[3] ^ x[20]
vmov s8, r6    @ spill x[8] from r6
vmov r6, s21    @ load x[21] from s21
eor r7, r7, r6    @ x[24] = x[24] ^ x[21]
vmov s14, r4    @ spill x[14] from r4
vmov r4, s2    @ load x[2] from s2
eor r4, r4, r5    @ x[2] = x[2] ^ x[20]
vmov s2, r4    @ spill x[2] from r4
vmov r4, s12    @ load x[12] from s12
eor r4, r4, r5    @ x[12] = x[12] ^ x[20]
eor r1, r1, r6    @ x[11] = x[11] ^ x[21]
vmov s6, r10    @ spill x[6] from r10
vmov r10, s9    @ load x[9] from s9
eor r10, r10, r0    @ x[9] = x[9] ^ x[29]
eor r12, r12, r3    @ x[3] = x[3] ^ x[31]
eor r0, r0, r7    @ x[29] = x[29] ^ x[24]
vmov s29, r0    @ spill x[29] from r0
vmov r0, s13    @ load x[13] from s13
vmov s9, r10    @ spill x[9] from r10
vmov r10, s15    @ load x[15] from s15
eor r0, r0, r10    @ x[13] = x[13] ^ x[15]
eor r10, r10, r14    @ x[15] = x[15] ^ x[4]
vmov s13, r0    @ spill x[13] from r0
vmov r0, s16    @ load x[16] from s16
eor r14, r14, r0    @ x[4] = x[4] ^ x[16]
eor r1, r1, r14    @ x[11] = x[11] ^ x[4]
vmov s11, r1    @ spill x[11] from r1
vmov r1, s19    @ load x[19] from s19
eor r7, r7, r1    @ x[24] = x[24] ^ x[19]
eor r3, r3, r1    @ x[31] = x[31] ^ x[19]
eor r9, r9, r11    @ x[10] = x[10] ^ x[22]
vmov s10, r9    @ spill x[10] from r9
vmov r9, s17    @ load x[17] from s17
eor r4, r4, r9    @ x[12] = x[12] ^ x[17]
eor r3, r3, r9    @ x[31] = x[31] ^ x[17]
vmov s12, r4    @ spill x[12] from r4
vmov r4, s27    @ load x[27] from s27
eor r9, r9, r4    @ x[17] = x[17] ^ x[27]
eor r5, r5, r4    @ x[20] = x[20] ^ x[27]
eor r11, r11, r4    @ x[22] = x[22] ^ x[27]
eor r1, r1, r8    @ x[19] = x[19] ^ x[30]
vmov s19, r1    @ spill x[19] from r1
vmov r1, s25    @ load x[25] from s25
eor r4, r4, r1    @ x[27] = x[27] ^ x[25]
eor r8, r8, r1    @ x[30] = x[30] ^ x[25]
vmov s24, r7    @ spill x[24] from r7
vmov r7, s7    @ load x[7] from s7
vmov s28, r2    @ spill x[28] from r2
vmov r2, s23    @ load x[23] from s23
eor r7, r7, r2    @ x[7] = x[7] ^ x[23]
eor r1, r1, r2    @ x[25] = x[25] ^ x[23]
eor r2, r2, r6    @ x[23] = x[23] ^ x[21]
vmov s4, r14    @ spill x[4] from r14
vmov r14, s1    @ load x[1] from s1
eor r14, r14, r1    @ x[1] = x[1] ^ x[25]
eor r6, r6, r4    @ x[21] = x[21] ^ x[27]
eor r10, r10, r5    @ x[15] = x[15] ^ x[20]
eor r8, r8, r3    @ x[30] = x[30] ^ x[31]
eor r7, r7, r11    @ x[7] = x[7] ^ x[22]
eor r1, r1, r9    @ x[25] = x[25] ^ x[17]
eor r0, r0, r2    @ x[16] = x[16] ^ x[23]
vmov s30, r8    @ spill x[30] from r8
vmov r8, s4    @ load x[4] from s4
eor r8, r8, r12    @ x[4] = x[4] ^ x[3]
eor r4, r4, r3    @ x[27] = x[27] ^ x[31]
vmov s16, r0    @ spill x[16] from r0
vmov r0, s26    @ load x[26] from s26
eor r0, r0, r3    @ x[26] = x[26] ^ x[31]
vmov s31, r3    @ spill x[31] from r3
vmov r3, s28    @ load x[28] from s28
eor r3, r3, r0    @ x[28] = x[28] ^ x[26]
vmov s17, r9    @ spill x[17] from r9
vmov r9, s24    @ load x[24] from s24
eor r9, r9, r3    @ x[24] = x[24] ^ x[28]
eor r5, r5, r1    @ x[20] = x[20] ^ x[25]
vmov s20, r5    @ spill x[20] from r5
vmov r5, s0    @ load x[0] from s0
vmov s27, r4    @ spill x[27] from r4
vmov r4, s19    @ load x[19] from s19
eor r5, r5, r4    @ x[0] = x[0] ^ x[19]
vmov s25, r1    @ spill x[25] from r1
vmov r1, s6    @ load x[6] from s6
eor r1, r1, r11    @ x[6] = x[6] ^ x[22]
eor r6, r6, r4    @ x[21] = x[21] ^ x[19]
eor r12, r12, r6    @ x[3] = x[3] ^ x[21]
vmov s3, r12    @ spill x[3] from r12
vmov r12, s9    @ load x[9] from s9
vmov s22, r11    @ spill x[22] from r11
vmov r11, s13    @ load x[13] from s13
eor r12, r12, r11    @ x[9] = x[9] ^ x[13]
vmov s21, r6    @ spill x[21] from r6
vmov r6, s10    @ load x[10] from s10
eor r11, r11, r6    @ x[13] = x[13] ^ x[10]
vmov s7, r7    @ spill x[7] from r7
vmov r7, s14    @ load x[14] from s14
eor r7, r7, r8    @ x[14] = x[14] ^ x[4]
eor r10, r10, r12    @ x[15] = x[15] ^ x[9]
vmov s26, r0    @ evict x[26] from r0 before restoring y_base
pop {r0}    @ restore y base pointer
str  r5, [r0, #0]    @ y[0] = x[0] from r5
vstr.32  s16, [r0, #4]    @ y[1] = x[16] from s16
str  r6, [r0, #8]    @ y[2] = x[10] from r6
str  r2, [r0, #12]    @ y[3] = x[23] from r2
str  r11, [r0, #16]    @ y[4] = x[13] from r11
vstr.32  s17, [r0, #20]    @ y[5] = x[17] from s17
vstr.32  s3, [r0, #24]    @ y[6] = x[3] from s3
str  r4, [r0, #28]    @ y[7] = x[19] from r4
str  r10, [r0, #32]    @ y[8] = x[15] from r10
vstr.32  s20, [r0, #36]    @ y[9] = x[20] from s20
str  r8, [r0, #40]    @ y[10] = x[4] from r8
vstr.32  s30, [r0, #44]    @ y[11] = x[30] from s30
vstr.32  s7, [r0, #48]    @ y[12] = x[7] from s7
vstr.32  s29, [r0, #52]    @ y[13] = x[29] from s29
vstr.32  s2, [r0, #56]    @ y[14] = x[2] from s2
vstr.32  s18, [r0, #60]    @ y[15] = x[18] from s18
vstr.32  s5, [r0, #64]    @ y[16] = x[5] from s5
vstr.32  s21, [r0, #68]    @ y[17] = x[21] from s21
vstr.32  s11, [r0, #72]    @ y[18] = x[11] from s11
vstr.32  s27, [r0, #76]    @ y[19] = x[27] from s27
str  r14, [r0, #80]    @ y[20] = x[1] from r14
str  r3, [r0, #84]    @ y[21] = x[28] from r3
str  r12, [r0, #88]    @ y[22] = x[9] from r12
vstr.32  s25, [r0, #92]    @ y[23] = x[25] from s25
vstr.32  s8, [r0, #96]    @ y[24] = x[8] from s8
vstr.32  s26, [r0, #100]    @ y[25] = x[26] from s26
str  r1, [r0, #104]    @ y[26] = x[6] from r1
vstr.32  s22, [r0, #108]    @ y[27] = x[22] from s22
vstr.32  s12, [r0, #112]    @ y[28] = x[12] from s12
str  r9, [r0, #116]    @ y[29] = x[24] from r9
str  r7, [r0, #120]    @ y[30] = x[14] from r7
vstr.32  s31, [r0, #124]    @ y[31] = x[31] from s31
pop     {r1}
vpop    { d8-d15 }
add     sp, sp, #4
ldmia   sp!, {r4-r11, pc}
.size   decode_from_gft, .-decode_from_gft
.syntax unified
.cpu cortex-m4
.fpu fpv4-sp-d16
.global encode_to_gft_full
.type encode_to_gft_full, %function
.align 2
encode_to_gft_full:
stmdb   sp!, {r4-r11, lr}
sub     sp, sp, #4
vpush   {d8-d15}
vldr.32 s0, [r1, #0]
vldr.32 s1, [r1, #4]
vldr.32 s2, [r1, #8]
vldr.32 s3, [r1, #12]
vldr.32 s4, [r1, #16]
vldr.32 s5, [r1, #20]
vldr.32 s6, [r1, #24]
vldr.32 s7, [r1, #28]
vldr.32 s8, [r1, #32]
vldr.32 s9, [r1, #36]
vldr.32 s10, [r1, #40]
vldr.32 s11, [r1, #44]
vldr.32 s12, [r1, #48]
vldr.32 s13, [r1, #52]
vldr.32 s14, [r1, #56]
vldr.32 s15, [r1, #60]
vldr.32 s16, [r1, #64]
vldr.32 s17, [r1, #68]
vldr.32 s18, [r1, #72]
vldr.32 s19, [r1, #76]
vldr.32 s20, [r1, #80]
vldr.32 s21, [r1, #84]
vldr.32 s22, [r1, #88]
vldr.32 s23, [r1, #92]
vldr.32 s24, [r1, #96]
vldr.32 s25, [r1, #100]
vldr.32 s26, [r1, #104]
vldr.32 s27, [r1, #108]
vldr.32 s28, [r1, #112]
vldr.32 s29, [r1, #116]
vldr.32 s30, [r1, #120]
vldr.32 s31, [r1, #124]
push {r0, r1}
vmov r0, s15    @ load x[15] from s15
vmov r1, s31    @ load x[31] from s31
eor r0, r0, r1    @ x[15] = x[15] ^ x[31]
vmov r2, s23    @ load x[23] from s23
vmov r3, s27    @ load x[27] from s27
eor r2, r2, r3    @ x[23] = x[23] ^ x[27]
vmov r4, s7    @ load x[7] from s7
vmov r5, s9    @ load x[9] from s9
eor r4, r4, r5    @ x[7] = x[7] ^ x[9]
vmov r6, s16    @ load x[16] from s16
vmov r7, s4    @ load x[4] from s4
eor r6, r6, r7    @ x[16] = x[16] ^ x[4]
vmov r8, s25    @ load x[25] from s25
eor r6, r6, r8    @ x[16] = x[16] ^ x[25]
vmov r9, s8    @ load x[8] from s8
eor r9, r9, r6    @ x[8] = x[8] ^ x[16]
vmov r10, s0    @ load x[0] from s0
vmov r11, s17    @ load x[17] from s17
eor r10, r10, r11    @ x[0] = x[0] ^ x[17]
vmov r12, s5    @ load x[5] from s5
eor r11, r11, r12    @ x[17] = x[17] ^ x[5]
vmov r14, s2    @ load x[2] from s2
vmov s27, r3    @ spill x[27] from r3
vmov r3, s1    @ load x[1] from s1
eor r14, r14, r3    @ x[2] = x[2] ^ x[1]
eor r14, r14, r9    @ x[2] = x[2] ^ x[8]
vmov s31, r1    @ spill x[31] from r1
vmov r1, s13    @ load x[13] from s13
eor r5, r5, r1    @ x[9] = x[9] ^ x[13]
vmov s9, r5    @ spill x[9] from r5
vmov r5, s14    @ load x[14] from s14
vmov s13, r1    @ spill x[13] from r1
vmov r1, s20    @ load x[20] from s20
eor r5, r5, r1    @ x[14] = x[14] ^ x[20]
vmov s0, r10    @ spill x[0] from r10
vmov r10, s3    @ load x[3] from s3
eor r4, r4, r10    @ x[7] = x[7] ^ x[3]
vmov s7, r4    @ spill x[7] from r4
vmov r4, s11    @ load x[11] from s11
vmov s16, r6    @ spill x[16] from r6
vmov r6, s21    @ load x[21] from s21
eor r4, r4, r6    @ x[11] = x[11] ^ x[21]
eor r8, r8, r4    @ x[25] = x[25] ^ x[11]
eor r9, r9, r10    @ x[8] = x[8] ^ x[3]
vmov s25, r8    @ spill x[25] from r8
vmov r8, s18    @ load x[18] from s18
eor r7, r7, r8    @ x[4] = x[4] ^ x[18]
eor r8, r8, r6    @ x[18] = x[18] ^ x[21]
eor r9, r9, r12    @ x[8] = x[8] ^ x[5]
vmov s5, r12    @ spill x[5] from r12
vmov r12, s10    @ load x[10] from s10
vmov s8, r9    @ spill x[8] from r9
vmov r9, s30    @ load x[30] from s30
eor r12, r12, r9    @ x[10] = x[10] ^ x[30]
eor r8, r8, r9    @ x[18] = x[18] ^ x[30]
eor r9, r9, r3    @ x[30] = x[30] ^ x[1]
vmov s1, r3    @ spill x[1] from r3
vmov r3, s24    @ load x[24] from s24
eor r3, r3, r5    @ x[24] = x[24] ^ x[14]
vmov s30, r9    @ spill x[30] from r9
vmov r9, s22    @ load x[22] from s22
eor r9, r9, r2    @ x[22] = x[22] ^ x[23]
vmov s20, r1    @ spill x[20] from r1
vmov r1, s6    @ load x[6] from s6
eor r1, r1, r14    @ x[6] = x[6] ^ x[2]
eor r2, r2, r0    @ x[23] = x[23] ^ x[15]
vmov s6, r1    @ spill x[6] from r1
vmov r1, s12    @ load x[12] from s12
eor r5, r5, r1    @ x[14] = x[14] ^ x[12]
eor r1, r1, r0    @ x[12] = x[12] ^ x[15]
eor r7, r7, r11    @ x[4] = x[4] ^ x[17]
eor r6, r6, r11    @ x[21] = x[21] ^ x[17]
eor r9, r9, r8    @ x[22] = x[22] ^ x[18]
eor r12, r12, r14    @ x[10] = x[10] ^ x[2]
eor r3, r3, r14    @ x[24] = x[24] ^ x[2]
vmov s17, r11    @ spill x[17] from r11
vmov r11, s28    @ load x[28] from s28
eor r14, r14, r11    @ x[2] = x[2] ^ x[28]
eor r14, r14, r4    @ x[2] = x[2] ^ x[11]
eor r4, r4, r10    @ x[11] = x[11] ^ x[3]
vmov s21, r6    @ spill x[21] from r6
vmov r6, s29    @ load x[29] from s29
eor r10, r10, r6    @ x[3] = x[3] ^ x[29]
eor r7, r7, r14    @ x[4] = x[4] ^ x[2]
eor r0, r0, r4    @ x[15] = x[15] ^ x[11]
vmov s15, r0    @ spill x[15] from r0
vmov r0, s19    @ load x[19] from s19
eor r4, r4, r0    @ x[11] = x[11] ^ x[19]
eor r7, r7, r12    @ x[4] = x[4] ^ x[10]
vmov s11, r4    @ spill x[11] from r4
vmov r4, s20    @ load x[20] from s20
eor r4, r4, r12    @ x[20] = x[20] ^ x[10]
vmov s19, r0    @ spill x[19] from r0
vmov r0, s8    @ load x[8] from s8
eor r11, r11, r0    @ x[28] = x[28] ^ x[8]
eor r0, r0, r9    @ x[8] = x[8] ^ x[22]
eor r3, r3, r2    @ x[24] = x[24] ^ x[23]
vmov s23, r2    @ spill x[23] from r2
vmov r2, s26    @ load x[26] from s26
vmov s24, r3    @ spill x[24] from r3
vmov r3, s30    @ load x[30] from s30
eor r2, r2, r3    @ x[26] = x[26] ^ x[30]
eor r8, r8, r2    @ x[18] = x[18] ^ x[26]
eor r3, r3, r11    @ x[30] = x[30] ^ x[28]
eor r12, r12, r1    @ x[10] = x[10] ^ x[12]
vmov s26, r2    @ spill x[26] from r2
vmov r2, s16    @ load x[16] from s16
eor r1, r1, r2    @ x[12] = x[12] ^ x[16]
vmov s12, r1    @ spill x[12] from r1
vmov r1, s1    @ load x[1] from s1
vmov s30, r3    @ spill x[30] from r3
vmov r3, s7    @ load x[7] from s7
eor r1, r1, r3    @ x[1] = x[1] ^ x[7]
eor r2, r2, r6    @ x[16] = x[16] ^ x[29]
eor r2, r2, r0    @ x[16] = x[16] ^ x[8]
eor r0, r0, r5    @ x[8] = x[8] ^ x[14]
eor r5, r5, r12    @ x[14] = x[14] ^ x[10]
vmov s14, r5    @ spill x[14] from r5
vmov r5, s6    @ load x[6] from s6
eor r2, r2, r5    @ x[16] = x[16] ^ x[6]
vmov s8, r0    @ spill x[8] from r0
vmov r0, s0    @ load x[0] from s0
eor r0, r0, r9    @ x[0] = x[0] ^ x[22]
eor r11, r11, r6    @ x[28] = x[28] ^ x[29]
eor r0, r0, r14    @ x[0] = x[0] ^ x[2]
eor r12, r12, r7    @ x[10] = x[10] ^ x[4]
eor r7, r7, r3    @ x[4] = x[4] ^ x[7]
vmov s0, r0    @ spill x[0] from r0
vmov r0, s13    @ load x[13] from s13
eor r8, r8, r0    @ x[18] = x[18] ^ x[13]
vmov s28, r11    @ spill x[28] from r11
vmov r11, s21    @ load x[21] from s21
eor r2, r2, r11    @ x[16] = x[16] ^ x[21]
eor r1, r1, r10    @ x[1] = x[1] ^ x[3]
eor r10, r10, r0    @ x[3] = x[3] ^ x[13]
eor r5, r5, r0    @ x[6] = x[6] ^ x[13]
vmov s1, r1    @ spill x[1] from r1
vmov r1, s17    @ load x[17] from s17
eor r0, r0, r1    @ x[13] = x[13] ^ x[17]
eor r12, r12, r4    @ x[10] = x[10] ^ x[20]
eor r4, r4, r8    @ x[20] = x[20] ^ x[18]
vmov s10, r12    @ spill x[10] from r12
vmov r12, s19    @ load x[19] from s19
eor r8, r8, r12    @ x[18] = x[18] ^ x[19]
eor r9, r9, r10    @ x[22] = x[22] ^ x[3]
vmov s13, r0    @ spill x[13] from r0
vmov r0, s31    @ load x[31] from s31
eor r10, r10, r0    @ x[3] = x[3] ^ x[31]
eor r1, r1, r3    @ x[17] = x[17] ^ x[7]
vmov s17, r1    @ spill x[17] from r1
vmov r1, s5    @ load x[5] from s5
eor r2, r2, r1    @ x[16] = x[16] ^ x[5]
vmov s16, r2    @ spill x[16] from r2
vmov r2, s11    @ load x[11] from s11
eor r1, r1, r2    @ x[5] = x[5] ^ x[11]
eor r9, r9, r4    @ x[22] = x[22] ^ x[20]
vmov s21, r11    @ spill x[21] from r11
vmov r11, s30    @ load x[30] from s30
eor r4, r4, r11    @ x[20] = x[20] ^ x[30]
vmov s20, r4    @ spill x[20] from r4
vmov r4, s12    @ load x[12] from s12
eor r8, r8, r4    @ x[18] = x[18] ^ x[12]
eor r10, r10, r1    @ x[3] = x[3] ^ x[5]
eor r14, r14, r7    @ x[2] = x[2] ^ x[4]
eor r7, r7, r5    @ x[4] = x[4] ^ x[6]
vmov s3, r10    @ spill x[3] from r10
vmov r10, s24    @ load x[24] from s24
eor r14, r14, r10    @ x[2] = x[2] ^ x[24]
eor r10, r10, r11    @ x[24] = x[24] ^ x[30]
vmov s2, r14    @ spill x[2] from r14
vmov r14, s27    @ load x[27] from s27
eor r9, r9, r14    @ x[22] = x[22] ^ x[27]
eor r11, r11, r12    @ x[30] = x[30] ^ x[19]
eor r10, r10, r5    @ x[24] = x[24] ^ x[6]
eor r11, r11, r0    @ x[30] = x[30] ^ x[31]
eor r12, r12, r14    @ x[19] = x[19] ^ x[27]
eor r14, r14, r2    @ x[27] = x[27] ^ x[11]
eor r0, r0, r3    @ x[31] = x[31] ^ x[7]
eor r0, r0, r6    @ x[31] = x[31] ^ x[29]
vmov s4, r7    @ spill x[4] from r7
vmov r7, s8    @ load x[8] from s8
eor r11, r11, r7    @ x[30] = x[30] ^ x[8]
eor r14, r14, r6    @ x[27] = x[27] ^ x[29]
eor r2, r2, r3    @ x[11] = x[11] ^ x[7]
vmov s22, r9    @ spill x[22] from r9
vmov r9, s9    @ load x[9] from s9
eor r6, r6, r9    @ x[29] = x[29] ^ x[9]
vmov s24, r10    @ spill x[24] from r10
vmov r10, s25    @ load x[25] from s25
eor r3, r3, r10    @ x[7] = x[7] ^ x[25]
eor r1, r1, r10    @ x[5] = x[5] ^ x[25]
eor r7, r7, r9    @ x[8] = x[8] ^ x[9]
vmov s8, r7    @ spill x[8] from r7
vmov r7, s21    @ load x[21] from s21
eor r11, r11, r7    @ x[30] = x[30] ^ x[21]
vmov s30, r11    @ spill x[30] from r11
vmov r11, s20    @ load x[20] from s20
eor r11, r11, r9    @ x[20] = x[20] ^ x[9]
eor r5, r5, r1    @ x[6] = x[6] ^ x[5]
vmov s18, r8    @ spill x[18] from r8
vmov r8, s15    @ load x[15] from s15
eor r1, r1, r8    @ x[5] = x[5] ^ x[15]
vmov s20, r11    @ spill x[20] from r11
vmov r11, s14    @ load x[14] from s14
eor r4, r4, r11    @ x[12] = x[12] ^ x[14]
eor r3, r3, r7    @ x[7] = x[7] ^ x[21]
vmov s7, r3    @ spill x[7] from r3
vmov r3, s23    @ load x[23] from s23
eor r2, r2, r3    @ x[11] = x[11] ^ x[23]
vmov s5, r1    @ spill x[5] from r1
vmov r1, s13    @ load x[13] from s13
eor r5, r5, r1    @ x[6] = x[6] ^ x[13]
eor r7, r7, r3    @ x[21] = x[21] ^ x[23]
eor r8, r8, r9    @ x[15] = x[15] ^ x[9]
eor r9, r9, r12    @ x[9] = x[9] ^ x[19]
eor r12, r12, r1    @ x[19] = x[19] ^ x[13]
eor r1, r1, r3    @ x[13] = x[13] ^ x[23]
eor r3, r3, r6    @ x[23] = x[23] ^ x[29]
eor r10, r10, r3    @ x[25] = x[25] ^ x[23]
eor r10, r10, r0    @ x[25] = x[25] ^ x[31]
vmov s19, r12    @ spill x[19] from r12
vmov r12, s26    @ load x[26] from s26
eor r12, r12, r11    @ x[26] = x[26] ^ x[14]
vmov s25, r10    @ spill x[25] from r10
vmov r10, s10    @ load x[10] from s10
eor r11, r11, r10    @ x[14] = x[14] ^ x[10]
eor r4, r4, r0    @ x[12] = x[12] ^ x[31]
eor r0, r0, r14    @ x[31] = x[31] ^ x[27]
eor r9, r9, r7    @ x[9] = x[9] ^ x[21]
eor r5, r5, r0    @ x[6] = x[6] ^ x[31]
eor r11, r11, r7    @ x[14] = x[14] ^ x[21]
vmov s6, r5    @ spill x[6] from r5
vmov r5, s17    @ load x[17] from s17
eor r7, r7, r5    @ x[21] = x[21] ^ x[17]
eor r10, r10, r6    @ x[10] = x[10] ^ x[29]
eor r0, r0, r8    @ x[31] = x[31] ^ x[15]
vmov s27, r14    @ spill x[27] from r14
vmov r14, s16    @ load x[16] from s16
eor r14, r14, r2    @ x[16] = x[16] ^ x[11]
vmov s11, r2    @ spill x[11] from r2
vmov r2, s28    @ load x[28] from s28
vmov s9, r9    @ spill x[9] from r9
vmov r9, s5    @ load x[5] from s5
eor r2, r2, r9    @ x[28] = x[28] ^ x[5]
vmov s15, r8    @ spill x[15] from r8
vmov r8, s7    @ load x[7] from s7
eor r6, r6, r8    @ x[29] = x[29] ^ x[7]
vmov s29, r6    @ spill x[29] from r6
vmov r6, s20    @ load x[20] from s20
eor r6, r6, r9    @ x[20] = x[20] ^ x[5]
eor r0, r0, r1    @ x[31] = x[31] ^ x[13]
vmov s10, r10    @ spill x[10] from r10
vmov r10, s18    @ load x[18] from s18
eor r10, r10, r1    @ x[18] = x[18] ^ x[13]
eor r9, r9, r3    @ x[5] = x[5] ^ x[23]
vmov s23, r3    @ spill x[23] from r3
vmov r3, s24    @ load x[24] from s24
eor r11, r11, r3    @ x[14] = x[14] ^ x[24]
vmov s13, r1    @ spill x[13] from r1
vmov r1, s30    @ load x[30] from s30
eor r1, r1, r10    @ x[30] = x[30] ^ x[18]
vmov s17, r5    @ spill x[17] from r5
vmov r5, s22    @ load x[22] from s22
eor r5, r5, r4    @ x[22] = x[22] ^ x[12]
eor r3, r3, r4    @ x[24] = x[24] ^ x[12]
vmov s12, r4    @ spill x[12] from r4
vmov r4, s4    @ load x[4] from s4
eor r4, r4, r5    @ x[4] = x[4] ^ x[22]
vmov s5, r9    @ spill x[5] from r9
vmov r9, s3    @ load x[3] from s3
eor r8, r8, r9    @ x[7] = x[7] ^ x[3]
eor r11, r11, r2    @ x[14] = x[14] ^ x[28]
eor r12, r12, r14    @ x[26] = x[26] ^ x[16]
eor r7, r7, r9    @ x[21] = x[21] ^ x[3]
vmov s21, r7    @ spill x[21] from r7
vmov r7, s1    @ load x[1] from s1
eor r7, r7, r0    @ x[1] = x[1] ^ x[31]
eor r2, r2, r5    @ x[28] = x[28] ^ x[22]
vmov s31, r0    @ spill x[31] from r0
vmov r0, s2    @ load x[2] from s2
eor r0, r0, r2    @ x[2] = x[2] ^ x[28]
eor r1, r1, r3    @ x[30] = x[30] ^ x[24]
vmov s2, r0    @ evict x[2] from r0 before restoring y_base
pop {r0}    @ restore y base pointer
vstr.32  s0, [r0, #0]    @ y[0] = x[0] from s0
str  r5, [r0, #4]    @ y[1] = x[22] from r5
vstr.32  s8, [r0, #8]    @ y[2] = x[8] from s8
vstr.32  s12, [r0, #12]    @ y[3] = x[12] from s12
vstr.32  s6, [r0, #16]    @ y[4] = x[6] from s6
str  r4, [r0, #20]    @ y[5] = x[4] from r4
str  r11, [r0, #24]    @ y[6] = x[14] from r11
str  r6, [r0, #28]    @ y[7] = x[20] from r6
str  r10, [r0, #32]    @ y[8] = x[18] from r10
str  r14, [r0, #36]    @ y[9] = x[16] from r14
vstr.32  s2, [r0, #40]    @ y[10] = x[2] from s2
str  r12, [r0, #44]    @ y[11] = x[26] from r12
str  r3, [r0, #48]    @ y[12] = x[24] from r3
str  r2, [r0, #52]    @ y[13] = x[28] from r2
vstr.32  s10, [r0, #56]    @ y[14] = x[10] from s10
str  r1, [r0, #60]    @ y[15] = x[30] from r1
str  r7, [r0, #64]    @ y[16] = x[1] from r7
vstr.32  s13, [r0, #68]    @ y[17] = x[13] from s13
vstr.32  s9, [r0, #72]    @ y[18] = x[9] from s9
vstr.32  s15, [r0, #76]    @ y[19] = x[15] from s15
vstr.32  s17, [r0, #80]    @ y[20] = x[17] from s17
vstr.32  s21, [r0, #84]    @ y[21] = x[21] from s21
str  r8, [r0, #88]    @ y[22] = x[7] from r8
vstr.32  s27, [r0, #92]    @ y[23] = x[27] from s27
vstr.32  s19, [r0, #96]    @ y[24] = x[19] from s19
vstr.32  s31, [r0, #100]    @ y[25] = x[31] from s31
vstr.32  s5, [r0, #104]    @ y[26] = x[5] from s5
vstr.32  s11, [r0, #108]    @ y[27] = x[11] from s11
vstr.32  s29, [r0, #112]    @ y[28] = x[29] from s29
vstr.32  s23, [r0, #116]    @ y[29] = x[23] from s23
str  r9, [r0, #120]    @ y[30] = x[3] from r9
vstr.32  s25, [r0, #124]    @ y[31] = x[25] from s25
pop     {r1}
vpop    { d8-d15 }
add     sp, sp, #4
ldmia   sp!, {r4-r11, pc}
.size   encode_to_gft_full, .-encode_to_gft_full
