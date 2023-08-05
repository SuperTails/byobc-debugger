DIGIT_BUF = $40

IO_DRB  = $6000
IO_DRA  = $6001
IO_DDRB = $6002
IO_DDRA = $6003

	.org $8000

start:
	ldx #$FF
	txs

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

	lda #124
	jsr convert_to_bcd
	jsr print_bcd

loop:
	jmp loop

print_bcd:
	ldx #$20
	ldy #$A0

	lda DIGIT_BUF+3
	clc
	adc #'0'
	sta IO_DRB
	sty IO_DRA
	stx IO_DRA

	lda DIGIT_BUF+2
	adc #'0'
	sta IO_DRB
	sty IO_DRA
	stx IO_DRA

	lda DIGIT_BUF+1
	adc #'0'
	sta IO_DRB
	sty IO_DRA
	stx IO_DRA

	lda #' '
	sta IO_DRB
	sty IO_DRA
	stx IO_DRA

	lda DIGIT_BUF+0
	jsr print_hex
	lda DIGIT_BUF+1
	jsr print_hex
	lda DIGIT_BUF+2
	jsr print_hex
	lda DIGIT_BUF+3
	jmp print_hex


print_hex:
	pha
	ldx #$20
	ldy #$A0
	lsr
	lsr
	lsr
	lsr
	cmp #$0A
	bpl .hex1
	adc #('0' - 'A' + $0A)
.hex1:
	clc
	adc #('A' - $0A)
	sta IO_DRB
	sty IO_DRA
	stx IO_DRA

	pla
	and #$0F
	cmp #$0A
	bpl .hex2
	adc #('0' - 'A' + $0A)
.hex2:
	clc
	adc #('A' - $0A)
	sta IO_DRB
	sty IO_DRA
	stx IO_DRA
	rts

convert_to_bcd:
	pha
.init:
	sta DIGIT_BUF+0
	ldx #$00
	stx DIGIT_BUF+1
	stx DIGIT_BUF+2
	stx DIGIT_BUF+3
	ldx #$08
.loop:
	lda DIGIT_BUF+1
	cmp #5
	bmi .mid_digit
	adc #(128-5-1)		; -1 accounts for carry bit
	sta DIGIT_BUF+1
.mid_digit:
	lda DIGIT_BUF+2
	cmp #5
	bmi .shift
	adc #(128-5-1)		; -1 accounts for carry bit
	sta DIGIT_BUF+2
.shift:
	asl DIGIT_BUF+0
	rol DIGIT_BUF+1
	rol DIGIT_BUF+2
	rol DIGIT_BUF+3

	dex
	bne .loop
.exit:
	pla
	rts

do_thing:
	sta IO_DRB
	sty IO_DRA
	stx IO_DRA
	rts
	
	.org $FFFA
reset_vectors:
	.word 0x8000	; NMI
	.word start		; RESET
	.word 0x8000	; IRQ
