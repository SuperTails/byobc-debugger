
	.org $8000

_start:
	lda #$24
	sta $17
	ldx $17
	jmp _start

	.org $FFFA
	.word 0x8000
	.word 0x8000
	.word 0x8000
