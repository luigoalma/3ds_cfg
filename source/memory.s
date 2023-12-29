	.arch armv6k
	.cpu  mpcore
	.arm
	.section .text.memset, "ax", %progbits
	.align  2
	.global memset
	.syntax unified
	.type   memset, %function
memset:
	@ r0 = dest
	@ r1 = value
	@ r2 = length
	cmp     r2, #0
	bxeq    lr
	push    {r0,r4-r8,lr}
	@ alignment check and fix
	mov     r3, r0, LSL#30
	beq     .L1
.L0:
	strb    r1, [r0], #1
	subs    r2, r2, #1
	popeq   {r0,r4-r8,pc}
	adds    r3, r3, #0x40000000
	bne     .L0
.L1: @ memset only uses the lowest byte of the data argument
	and     r1, r1, #0xFF
	orr     r1, r1, r1, LSL#8
	orr     r1, r1, r1, LSL#16
	mov     r12, r1
	movs    r3, r2, LSR#5
	beq     .L3
	@ preparing loop
	push    {r1, r12}
	push    {r1, r12}
	ldm     sp, {r4-r5}
	ldm     sp!, {r6-r8,lr}
.L2: @ set 32 bytes per loop!
	stm     r0!, {r1,r4-r8,r12,lr}
	subs    r3, r3, #1
	bne     .L2
	ands    r2, r2, #0x1F
	popeq   {r0,r4-r8,pc}
.L3:
	movs    r3, r2, LSR#2
	beq     .L6
.L4:
	str     r1, [r0], #4
	subs    r3, r3, #1
	bne     .L4
	ands    r2, r2, #3
.L5:
	popeq   {r0,r4-r8,pc}
.L6:
	strb    r1, [r0], #1
	subs    r2, r2, #1
	b       .L5
	.size   memset, .-memset

	.section .text.memcpy, "ax", %progbits
	.align  2
	.global memcpy
	.syntax unified
	.type   memcpy, %function
memcpy:
	@ r0 = dest
	@ r1 = src
	@ r2 = length
	@ check if length 0 and return if so
	cmp     r2, #0
	bxeq    lr
	push    {r0,r4-r9,lr}
	@ pre-fetch data
	pld     [r1]
	@ alignment check with word size
	@ if not aligned but both are in the same misalignment, fix it up
	@ otherwise jump to basic loop
	orr     r12, r0, r1
	ands    r12, r12, #3
	beq     .L8
	mov     r12, r0, LSL#30
	cmp     r12, r1, LSL#30
	bne     .L13
.L7:
	ldrb    r3, [r1], #1
	strb    r3, [r0], #1
	subs    r2, r2, #1
	popeq   {r0,r4-r9,pc}
	adds    r12, r12, #0x40000000
	bne     .L7
.L8:
	@ check if length higher than 32
	@ if so, do the 32 byte block copy loop,
	@ until there's nothing left or remainder to copy is less than 32
	movs    r3, r2, LSR#5
	beq     .L10
.L9:
	ldm     r1!, {r4-r9,r12,lr}
	stm     r0!, {r4-r9,r12,lr}
	subs    r3, r3, #1
	bne     .L9
	ands    r2, r2, #0x1F
	popeq   {r0,r4-r9,pc}
.L10:
	@ copy in word size the remaining data,
	@ and finish off with basic loop if can't copy all by word size.
	movs    r3, r2, LSR#2
	beq     .L13
.L11:
	ldr     r12, [r1], #4
	str     r12, [r0], #4
	subs    r3, r3, #1
	bne     .L11
	ands    r2, r2, #0x3
.L12: @ the basic loop
	popeq   {r0,r4-r9,pc}
.L13:
	ldrb    r3, [r1], #1
	strb    r3, [r0], #1
	subs    r2, r2, #1
	b       .L12
	.size   memcpy, .-memcpy

	.section .text.memmove, "ax", %progbits
	.align  2
	.global memmove
	.syntax unified
	.type   memmove, %function
memmove:
	@ r0 = dest
	@ r1 = src
	@ r2 = length
	cmp     r2, #0
	bxeq    lr
	cmp     r1, r0
	bhs     memcpy
	add     r3, r1, r2
	cmp     r0, r3
	bhs     memcpy
	push    {r4-r9,lr}
	add     r0, r0, r2
	mov     r1, r3
	@ alignment check with word size
	@ if not aligned but both are in the same misalignment, fix it up
	@ otherwise jump to basic loop
	orr     r12, r0, r1
	ands    r12, r12, #3
	beq     .L15
	mov     r12, r0, LSL#30
	cmp     r12, r1, LSL#30
	bne     .L20
.L14:
	ldrb    r3, [r1, #-1]!
	strb    r3, [r0, #-1]!
	subs    r2, r2, #1
	popeq   {r4-r9,pc}
	subs    r12, r12, #0x40000000
	bne     .L14
.L15:
	@ check if length higher than 32
	@ if so, do the 32 byte block copy loop,
	@ until there's nothing left or remainder to copy is less than 32
	movs    r3, r2, LSR#5
	beq     .L17
.L16:
	ldmdb   r1!, {r4-r9,r12,lr}
	stmdb   r0!, {r4-r9,r12,lr}
	subs    r3, r3, #1
	bne     .L16
	ands    r2, r2, #0x1F
	popeq   {r4-r9,pc}
.L17:
	@ copy in word size the remaining data,
	@ and finish off with basic loop if can't copy all by word size.
	movs    r3, r2, LSR#2
	beq     .L20
.L18:
	ldr     r12, [r1, #-4]!
	str     r12, [r0, #-4]!
	subs    r3, r3, #1
	bne     .L18
	ands    r2, r2, #0x3
.L19: @ the basic loop
	popeq   {r4-r9,pc}
.L20:
	ldrb    r3, [r1, #-1]!
	strb    r3, [r0, #-1]!
	subs    r2, r2, #1
	b       .L20
	.size   memmove, .-memmove

	.section .text.memcmp, "ax", %progbits
	.align  2
	.global memcmp
	.syntax unified
	.type   memcmp, %function
memcmp:
	@ r0 = ptr1
	@ r1 = ptr2
	@ r2 = length
	cmp     r2, #0
	moveq   r0, #0
	bxeq    lr
	push    {lr}
	mov     r12, r0
	@ alignment check with word size
	@ if not aligned but both are in the same misalignment, fix it up
	@ otherwise jump to basic loop
	orr     lr, r0, r1
	ands    lr, lr, #3
	beq     .L22
	mov     lr, r0, LSL#30
	cmp     lr, r1, LSL#30
	bne     .L25
.L21:
	ldrb    r0, [r12], #1
	ldrb    r3, [r1], #1
	subs    r0, r0, r3
	popne   {pc}
	subs    r2, r2, #1
	popeq   {pc}
	adds    lr, lr, #0x40000000
	bne     .L21
.L22:
	@ compare in word size.
	movs    r3, r2, LSR#2
	beq     .L25
.L23:
	ldr     r0, [r12], #4
	ldr     lr, [r1], #4
	subs    r0, r0, lr
	popne   {pc}
	subs    r3, r3, #1
	bne     .L23
	ands    r2, r2, #3
.L24: @ the basic loop
	popeq   {pc}
.L25:
	ldrb    r0, [r12], #1
	ldrb    r3, [r1], #1
	subs    r0, r0, r3
	popne   {pc}
	subs    r2, r2, #1
	b       .L24
	.size   memcmp, .-memcmp
