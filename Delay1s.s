.syntax unified
.cpu cortex-m4
.thumb

.equ BOUND_1s_X, 100
.equ BOUND_1s_Y, 10000
.global Delay_1s

Delay_1s:
	// a delay 1 sec function
	ldr r3,=BOUND_1s_X
L1_1s:
	ldr r5,=BOUND_1s_Y
L2_1s:
	subs r5,r5,#1
	bne L2_1s
	subs r3,r3,1
	bne L1_1s
	bx lr