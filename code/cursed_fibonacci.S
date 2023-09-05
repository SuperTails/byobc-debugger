	PROCESSOR 65C02

	SEG.U	variables
	ORG	$0000

	; there is not actually any memory

	SEG		text
	ORG $8000

start:
	lda #$01
	ldx #$00

fib_loop:
	tay
	clc
	adc value_table,X

	sta $00

	tax
	clc
	adc value_table,Y

	sta $00

	jmp fib_loop

value_table:
TABLE_VALUE SET 0
	REPEAT 256
	DC.B TABLE_VALUE
TABLE_VALUE SET TABLE_VALUE+1
	REPEND

	ORG $FFFA
reset_vectors:
	DC.W start	; NMI
	DC.W start	; RESET
	DC.W start	; IRQ
