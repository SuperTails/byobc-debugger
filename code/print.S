	PROCESSOR 65C02

VIA_REG_B	= $6000
VIA_REG_A	= $6001
VIA_DDR_B	= $6002
VIA_DDR_A	= $6003

LCD_EN_LOW		= (0 << 7)
LCD_EN_HIGH		= (1 << 7)

LCD_RW_WRITE	= (0 << 6)
LCD_RW_READ 	= (1 << 6)

LCD_RS_CMD		= (0 << 5)
LCD_RS_DATA		= (1 << 5)

; ========================================

	SEG		text
	ORG $8000

start:
	LDA #$FF
	STA VIA_DDR_A
	STA VIA_DDR_B

	LDX #[LCD_EN_HIGH | LCD_RW_WRITE | LCD_RS_CMD]
	LDY #[LCD_EN_LOW  | LCD_RW_WRITE | LCD_RS_CMD]

	LDA #%00111100			; 8-bit width, 2-line display, 5x10 font size
	STA VIA_REG_B			; Set the data lines to the correct command
	STX VIA_REG_A			; Turn enable on, set control signals
	STY VIA_REG_A			; Turn enable off

	LDA #%00000001			; Clear display
	STA VIA_REG_B
	STX VIA_REG_A
	STY VIA_REG_A

	LDA #%00000110			; Increment DDRAM pointer, do not shift display
	STA VIA_REG_B
	STX VIA_REG_A
	STY VIA_REG_A

	LDA #%00001111			; Set display on, cursor on, enable cursor blink
	STA VIA_REG_B
	STX VIA_REG_A
	STY VIA_REG_A

	LDX #[LCD_EN_HIGH | LCD_RW_WRITE | LCD_RS_DATA]
	LDY #[LCD_EN_LOW  | LCD_RW_WRITE | LCD_RS_DATA]

	LDA #'A			; This loads the ASCII value of the letter 'A', which is 65 decimal or $41 hex.
print_a:
	STA VIA_REG_B	; Set the data lines to the character we want to show
	STX VIA_REG_A	; Turn enable on, set control signals
	STY VIA_REG_A	; Turn enable off

	jmp print_a		; Print the letter A forever


	ORG $FFFA
reset_vectors:
	DC.W start	; NMI
	DC.W start	; RESET
	DC.W start	; IRQ
