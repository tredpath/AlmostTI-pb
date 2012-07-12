@**: EMULib Emulation Library *********************************
@**:                                                         **
@**:                     LibARM-BPP32.asm                    **
@**:                                                         **
@**: This file contains optimized ARM assembler functions    **
@**: used to copy and process 32bpp images on ARM-based      **
@**: platforms such as Symbian/S60, Symbian/UIQ, and Maemo.  **
@**:                                                         **
@**: Copyright (C) Marat Fayzullin 2005-2009                 **
@**:     You are not allowed to distribute this software     **
@**:     commercially. Please, notify me, if you make any    **
@**:     changes to this file.                               **
@*************************************************************:
	.text


	.global	TELEVIZE0_32
	.global	TELEVIZE1_32
	.global	C256T120_32
	.global	C256T160_32
	.global	C256T176_32
	.global	C256T208_32
	.global	C256T240_32
	.global	C256T256_32
	.global	C240T320_32
	.global	C256T320_32
	.global	C256T352_32
	.global	C256T416_32
	.global	C256T512_32
	.global	C256T768_32

	@** M_LOADCONSTS32 ***********************************************
	@** Loads two constants used for pixel merging.                 **
	@** For 24BPP/32BPP: r12=0x007F7F7F, r14=0x00808080             **
	@*****************************************************************
	.macro M_LOADCONSTS32
	mov r12,#0x00007F
	orr r12,r12,#0x007F00
	orr r12,r12,#0x7F0000
	mov r14,#0x000080
	orr r14,r14,#0x008000
	orr r14,r14,#0x800000
	.endm

	@** M_MERGE32 ****************************************************
	@** Merge two 24/32bpp pixels into one. Trashes \src1 and \tmp. **
	@*****************************************************************
	.macro M_MERGE32 dst,src1,src2,tmp
	and \tmp,\src1,r12
	and \dst,\src2,r12
	add \tmp,\tmp,\dst
	and \tmp,r12,\tmp,lsr #1
	and \src1,\src1,r14
	and \dst,\src2,r14
	add \dst,\dst,\src1
	add \dst,\tmp,\dst,lsr #1
	.endm

C256T120_32:	stmdb r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}
	M_LOADCONSTS32
L120:	ldmia r1!,{r4,r5}
	M_MERGE32 r3,r4,r5,r11
	ldmia r1!,{r5,r6,r7,r8,r9,r10}
	M_MERGE32 r4,r5,r6,r11
	M_MERGE32 r5,r7,r8,r11
	M_MERGE32 r6,r9,r10,r11
	stmia r0!,{r3,r4,r5,r6}
	subs r2,r2,#8
	bhi L120
	ldmia r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r15}

C256T160_32:	stmdb r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}
	M_LOADCONSTS32
L160:	ldmia r1!,{r3,r4,r6,r7,r8,r10}
	M_MERGE32 r5,r3,r4,r11
	M_MERGE32 r9,r7,r8,r11
	stmia r0!,{r5,r6,r9,r10}	@ DST[0,1,2,3] = MERGE(0,1),SRC[2],MERGE(3,4),SRC[5]
	ldmia r1!,{r3,r4,r6,r7,r9}
	M_MERGE32 r5,r3,r4,r11
	M_MERGE32 r8,r6,r7,r11
	stmia r0!,{r5,r8,r9}		@ DST[4,5,6] = MERGE(6,7),MERGE(8,9),SRC[10]
	ldmia r1!,{r3,r4,r6,r7,r8}
	M_MERGE32 r5,r3,r4,r11
	M_MERGE32 r9,r7,r8,r11
	stmia r0!,{r5,r6,r9}		@ DST[7,8,9] = MERGE(11,12),SRC[13],MERGE(14,15)
	subs r2,r2,#16
	bhi L160
	ldmia r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r15}

C256T176_32:	stmdb r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}
	M_LOADCONSTS32
L176:	ldmia r1!,{r3,r4,r6,r7,r8,r9}
	M_MERGE32 r5,r3,r4,r11
	M_MERGE32 r10,r8,r9,r11
	stmia r0!,{r5,r6,r7,r10}	@ DST[0,1,2,3] = MERGE(0,1),SRC[2],SRC[3],MERGE(4,5)
	ldmia r1!,{r3,r4,r5,r6,r8,r9,r10}
	M_MERGE32 r7,r5,r6,r11
	stmia r0!,{r3,r4,r7,r8}	@ DST[4,5,6,7] = SRC[6],SRC[7],MERGE(8,9),SRC[10]
	M_MERGE32 r3,r9,r10,r11
	ldmia r1!,{r4,r5,r6}
	M_MERGE32 r7,r5,r6,r11
	stmia r0!,{r3,r4,r7}		@ DST[8,9,10] = MERGE(11,12),SRC[13],MERGE(14,15)
	subs r2,r2,#16
	bhi L176
	ldmia r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r15}

C256T208_32:	stmdb r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}
	M_LOADCONSTS32
L208:	ldmia r1!,{r3,r4,r5,r6,r7,r8,r9,r10}
	stmia r0!,{r3,r4,r5,r6}
	M_MERGE32 r3,r7,r8,r11
	stmia r0!,{r3,r9,r10}
	ldmia r1!,{r3,r4,r5,r6,r7,r8,r9,r10}
	str r3,[r0],#4
	M_MERGE32 r3,r4,r5,r11
	stmia r0!,{r3,r6,r7,r8}
	M_MERGE32 r3,r9,r10,r11
	str r3,[r0],#4
	subs r2,r2,#16
	bhi L208
	ldmia r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r15}

C256T240_32:	stmdb r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}
	M_LOADCONSTS32
L240:	ldmia r1!,{r3,r4,r5,r6,r7,r8,r9,r10}
	stmia r0!,{r3,r4,r5,r6,r7,r8,r9,r10}
	ldmia r1!,{r5,r6}
	M_MERGE32 r4,r5,r6,r11
	ldmia r1!,{r5,r6,r7,r8,r9,r10}
	stmia r0!,{r4,r5,r6,r7,r8,r9,r10}
	subs r2,r2,#16
	bhi L240
	ldmia r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r15}

C256T256_32:	stmdb r13!,{r4,r5,r6,r7,r8,r9,r10}
L256:	ldmia r1!,{r3,r4,r5,r6,r7,r8,r9,r10}
	stmia r0!,{r3,r4,r5,r6,r7,r8,r9,r10}
	subs r2,r2,#8
	bhi L256
	ldmia r13!,{r4,r5,r6,r7,r8,r9,r10}
	mov r15,r14

C256T320_32:	stmdb r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}
	M_LOADCONSTS32
L320:	ldmia r1!,{r3,r5,r6,r7,r8,r10}
	str r3,[r0],#4			@ DST[0] = SRC[0]
	M_MERGE32 r4,r3,r5,r11
	mov r3,r8
	M_MERGE32 r9,r3,r10,r11
	stmia r0!,{r4,r5,r6,r7,r8,r9,r10}	@ DST[1-7] = MERGE(0,1),SRC[1-4],MERGE(4,5),SRC[5]
	ldmia r1!,{r3,r4}
	stmia r0!,{r3,r4}			@ DST[8-9] = SRC[6-7]
	subs r2,r2,#8
	bhi L320
	ldmia r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r15}

C240T320_32:	stmdb r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}
	M_LOADCONSTS32
L240T320:	ldmia r1!,{r3,r4,r6,r7,r8,r10}
	M_MERGE32 r5,r4,r6,r11
	M_MERGE32 r9,r8,r10,r11
	stmia r0!,{r3,r4,r5,r6,r7,r8,r9,r10}	@ DST[0-7] = SRC[0-1],MERGE(1,2),SRC[2-4],MERGE(4,5),SRC[5]
	subs r2,r2,#6
	bhi L240T320
	ldmia r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r15}

C256T352_32:	stmdb r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}
	M_LOADCONSTS32
L352:	ldmia r1!,{r3,r5,r6,r7,r8,r9,r10}
	str r3,[r0],#4		@ DST[0]  = SRC[0]
	M_MERGE32 r4,r3,r5,r11
	stmia r0!,{r4,r5,r6,r7}	@ DST[1,2,3,4] = MERGE(0,1),SRC[1],SRC[2],SRC[3]
	M_MERGE32 r6,r7,r8,r11
	stmia r0!,{r6,r8,r9}		@ DST[5,6,7] = MERGE(3,4),SRC[4],SRC[5]
	M_MERGE32 r6,r9,r10,r11
	ldr r11,[r1],#4
	stmia r0!,{r6,r10,r11}	@ DST[8,9,10] = MERGE(5,6),SRC[6],SRC[7]
	subs r2,r2,#8
	bhi L352
	ldmia r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r15}

C256T416_32:	stmdb r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}
	M_LOADCONSTS32
L416:	ldmia r1!,{r3,r5,r6,r7,r8,r9,r10}
	str r3,[r0],#4		@ DST[0]      = SRC[0]
	M_MERGE32 r4,r3,r5,r11
	stmia r0!,{r4,r5}		@ DST[1,2]    = MERGE(0,1),SRC[1]
	M_MERGE32 r3,r5,r6,r11
	stmia r0!,{r3,r6,r7}		@ DST[3,4,5]  = MERGE(1,2),SRC[2],SRC[3]
	M_MERGE32 r3,r7,r8,r11
	stmia r0!,{r3,r8}		@ DST[6,7]    = MERGE(3,4),SRC[4]
	M_MERGE32 r3,r8,r9,r11
	stmia r0!,{r3,r9,r10}		@ DST[8,9,10] = MERGE(4,5),SRC[5],SRC[6]
	ldr r4,[r1],#4
	M_MERGE32 r3,r10,r4,r11
	stmia r0!,{r3,r4}		@ DST[11,12]  = MERGE(6,7),SRC[7]
	subs r2,r2,#8
	bhi L416
	ldmia r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r15}

C256T512_32:	stmdb r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}
L512:	ldmia r1!,{r3,r4,r5,r6,r7,r8,r9,r10}
	mov r14,r5
	mov r12,r5
	mov r11,r4
	mov r5,r4
	mov r4,r3
	stmia r0!,{r3,r4,r5,r11,r12,r14}
	mov r3,r6
	mov r4,r6
	mov r5,r7
	mov r6,r7
	mov r7,r8
	mov r12,r10
	mov r11,r10
	mov r10,r9
	stmia r0!,{r3,r4,r5,r6,r7,r8,r9,r10,r11,r12}
	subs r2,r2,#8
	bhi L512
	ldmia r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r15}

C256T768_32:	stmdb r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r14}
L768:	ldmia r1!,{r3,r4,r5,r6,r7,r8,r9,r10}
	mov r14,r3
	mov r12,r3
	stmia r0!,{r3,r12,r14}
	mov r14,r5
	mov r12,r5
	mov r11,r5
	mov r3,r4
	mov r5,r4
	stmia r0!,{r3,r4,r5,r11,r12,r14}
	mov r14,r7
	mov r12,r7
	mov r3,r6
	mov r4,r6
	stmia r0!,{r3,r4,r6,r7,r12,r14}
	mov r14,r10
	mov r12,r10
	mov r3,r8
	mov r4,r8
	mov r5,r8
	mov r6,r9
	mov r7,r9
	stmia r0!,{r3,r4,r5,r6,r7,r9,r10,r12,r14}
	subs r2,r2,#8
	bhi L768
	ldmia r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r12,r15}

TELEVIZE0_32:	stmdb r13!,{r4,r5,r6,r7,r8,r9,r10,r14}
	mov r14,#0x000F
	orr r14,r14,#0x0F00
	orr r14,r14,r14,lsl #8
LTV0:	ldmia r0,{r3,r4,r5,r6,r7,r8,r9,r10}
	and r2,r14,r3,lsr #4
	sub r3,r3,r2
	and r2,r14,r4,lsr #4
	sub r4,r4,r2
	and r2,r14,r5,lsr #4
	sub r5,r5,r2
	and r2,r14,r6,lsr #4
	sub r6,r6,r2
	and r2,r14,r7,lsr #4
	sub r7,r7,r2
	and r2,r14,r8,lsr #4
	sub r8,r8,r2
	and r2,r14,r9,lsr #4
	sub r9,r9,r2
	and r2,r14,r10,lsr #4
	sub r10,r10,r2
	subs r1,r1,#8
	stmia r0!,{r3,r4,r5,r6,r7,r8,r9,r10}
	bhi LTV0
	ldmia r13!,{r4,r5,r6,r7,r8,r9,r10,r15}

TELEVIZE1_32:	stmdb r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r14}
	mov r14,#0x000F
	orr r14,r14,#0x0F00
	orr r14,r14,r14,lsl #8
	mvn r11,#1
LTV1:	ldmia r0,{r3,r4,r5,r6,r7,r8,r9,r10}
	eor r2,r3,r11
	and r2,r14,r2,lsr #4
	add r3,r3,r2
	eor r2,r4,r11
	and r2,r14,r2,lsr #4
	add r4,r4,r2
	eor r2,r5,r11
	and r2,r14,r2,lsr #4
	add r5,r5,r2
	eor r2,r6,r11
	and r2,r14,r2,lsr #4
	add r6,r6,r2
	eor r2,r7,r11
	and r2,r14,r2,lsr #4
	add r7,r7,r2
	eor r2,r8,r11
	and r2,r14,r2,lsr #4
	add r8,r8,r2
	eor r2,r9,r11
	and r2,r14,r2,lsr #4
	add r9,r9,r2
	eor r2,r10,r11
	and r2,r14,r2,lsr #4
	add r10,r10,r2
	subs r1,r1,#8
	stmia r0!,{r3,r4,r5,r6,r7,r8,r9,r10}
	bhi LTV1
	ldmia r13!,{r4,r5,r6,r7,r8,r9,r10,r11,r15}

	
