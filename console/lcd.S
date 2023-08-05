
IO_DRB  = $6000
IO_DRA  = $6001
IO_DDRB = $6002
IO_DDRA = $6003

	.org $8000

start:
	ldx #$00
	txs

	dex
	stx IO_DDRA
	stx IO_DDRB

	ldx #$00	; RS = 0, R/W = 0, E = 0
	ldy #$80	; RS = 0, R/w = 0, E = 1

	stx IO_DRA

	lda #$01
	jsr do_thing

	lda #$38	; 8-bit interface, 2-line display
	jsr do_thing

	lda #$0E
	jsr do_thing

	lda #'a'
	sta IO_DRB
	
	ldx #$20
	ldy #$A0
loop:
	clc
	adc #$01
	sta IO_DRB

	sty IO_DRA
	stx IO_DRA
	jmp loop

padding:
	nop
	nop
	nop
	nop
	nop
	nop
	nop

do_thing:
	sta IO_DRB
	sty IO_DRA
	stx IO_DRA
	rts
	
	.org $FFFA
reset_vectors:
	.word 0x8000
	.word 0x8000
	.word 0x8000
