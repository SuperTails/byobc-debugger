
FIB_A = $00
FIB_B = $01

	.org $8000

start:
	ldx #$00
	txs

	; Initialize the variables to zero
	stx FIB_A
	inx
	stx FIB_B

	; tmp = a + b
	; a = b
	; b = tmp
fib_loop:
	lda FIB_A
	clc
	adc FIB_B
	
	ldx FIB_B
	stx FIB_A

	sta FIB_B

	jmp fib_loop
	
	.org $FFFA
reset_vectors:
	.word 0x8000
	.word 0x8000
	.word 0x8000
