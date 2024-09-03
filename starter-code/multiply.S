	PROCESSOR 6502

lhs_value	EQU 23
rhs_value	EQU 7

	SEG.U	variables
	ORG	$0000

; ========= Your Variables Go Here =======

lhs: DS.B 1
rhs: DS.B 1

; ========================================


	SEG		text
	ORG $8000

start:
	lda #lhs_value
	sta lhs

	lda #rhs_value
	sta rhs

; ========= Your Code Goes Here ========

multiply:
	lda #$00
multiply_loop:
	lsr lhs
	bcc mul_no_add
mul_add:
	clc
	adc rhs
mul_no_add:
	asl rhs
	bne multiply_loop

; ======================================

halt:
	sta $00
	jmp halt



sequence_rom:
	DC.B 0, 2, 5, 6, 1, 4, 9, 8, 3, 7


	ORG $FFFA
reset_vectors:
	DC.W start	; NMI
	DC.W start	; RESET
	DC.W start	; IRQ
