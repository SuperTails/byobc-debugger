	PROCESSOR 65C02

	SEG		text
	ORG $8000

start:
	; Here, we load the offset of each string into the X register,
	; and then call your subroutine three times.
	; We use [square brackets] because (parenthesis) actually have special behavior,
	; but these could really just be written as ldx #0, ldx #14, ldx #28.
	ldx #[string_1-string_data]
	jsr checksum
	sta $10
	ldx #[string_2-string_data]
	jsr checksum
	sta $11
	ldx #[string_3-string_data]
	jsr checksum
	sta $12

	; Now load all the values we got to make sure you can see them in the debugger
	lda $10
	ldx $11
	ldy $12
halt:
	jmp halt

checksum:
	; When this function is called, the string you want to checksum
	; will be located at string_data + X.
	;
	; Your goal is to add up all the bytes in the string.
	; When the function returns, the sum should be in A.
	;
	; You will have to check for a null character (zero) at the end of the string.
	;
	; This can be done in 7 instructions.
	;
	; Fill in your code below!







	; Here's your string data!
string_data:
string_1:
	DC.B "hello, world!", 0
string_2:
	DC.B "hallo, world.", 0
string_3:
	DC.B "goodbye, world?", 0

	ORG $FFFA
reset_vectors:
	DC.W start	; NMI
	DC.W start	; RESET
	DC.W start	; IRQ
