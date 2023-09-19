	PROCESSOR 6502

	SEG text
	ORG $8000

start:
	ldx #10			; X will be your N, this counts down to 0.
	lda #0			; A should contain your running total and then final answer.

sum_loop:
	; Your code goes here!

	; Add X to A

	; Decrement X

	; Branch back up to the beginning of the loop!
	
halt:
	jmp halt 	; This is the end of the program; the processor will stay in this loop forever



	ORG $FFFA
reset_vectors:
	DC.W start	; NMI
	DC.W start	; RESET
	DC.W start	; IRQ
