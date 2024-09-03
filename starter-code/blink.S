	PROCESSOR 6502

	; Define some useful constants
VIA_REG_B	= $6000
VIA_REG_A	= $6001
VIA_DDR_B	= $6002
VIA_DDR_A	= $6003


	SEG		text
	ORG $8000

start:
	; First, make sure to set the pin to output

blink:
	; Now turn the LED on!

	; Now turn the LED off!

	JMP blink

	ORG $FFFA
vector_table:
	DC.W start	; NMI
	DC.W start	; RESET
	DC.W start	; IRQ
