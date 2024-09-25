.syntax unified
.cpu cortex-m4
.thumb

.equ GPIOC_IDR,  0x48000810
.equ BOUND_1s_X, 100
.equ BOUND_1s_Y, 10000
.global Delay_1s

Delay_1s:
	ldr r3,=BOUND_1s_X
	mov r8,#0;			// r8 would count for debounce;
	mov r9,#0;			// r9 would watch for if trigger
	mov r0,#0;
L1_1s:
	ldr r10,=BOUND_1s_Y
L2_1s:
	subs r10,r10,#1
	bne L2_1s

	trigger:
			cmp r8,#10
			beq d_tri

			ldr r2,=GPIOC_IDR;
			ldrh r1,[r2]
			and r1, #(1 << 13);
			cmp r1,#0
			beq add
			bne d_end
	add:
		add r8,r8,#1
		b d_end
	d_tri:
		mov r0,#1;
	d_end:
	subs r3,r3,1
	bne L1_1s

	bx lr
