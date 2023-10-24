	PROCESSOR 6502

VIA_REG_B	= $6000
VIA_REG_A	= $6001
VIA_DDR_B	= $6002
VIA_DDR_A	= $6003

ACIA_DATA 	= $5000
ACIA_STATUS	= $5001
ACIA_CMD	= $5002
ACIA_CTRL	= $5003


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

	; Now initialize the ACIA

	STA ACIA_STATUS			; Software reset

	LDA #%00010000			; 1 stop bit, 8-bit word length, use baud rate, 115200 baud
	STA ACIA_CTRL

	LDA #%00001011			; IRQB disabled, data terminal ready
	STA ACIA_CMD

	LDX #[LCD_EN_HIGH | LCD_RW_WRITE | LCD_RS_DATA]
	LDY #[LCD_EN_LOW  | LCD_RW_WRITE | LCD_RS_DATA]

	LDX #0
send_loop:
	LDA my_string,X
	BEQ done_sending
	STA ACIA_DATA
	INX
	JMP send_loop

done_sending:
	LDX #[LCD_EN_HIGH | LCD_RW_WRITE | LCD_RS_DATA]
	LDY #[LCD_EN_LOW  | LCD_RW_WRITE | LCD_RS_DATA]

recv_loop:
	LDA ACIA_STATUS
	AND #[1 << 3]
	BEQ recv_loop

	LDA ACIA_DATA
	STA VIA_REG_B

	STX VIA_REG_A	; Turn enable on, set control signals
	STY VIA_REG_A	; Turn enable off

	JMP recv_loop




	LDA #'A			; This loads the ASCII value of the letter 'A', which is 65 decimal or $41 hex.
print_a:
	STA VIA_REG_B	; Set the data lines to the character we want to show
	STX VIA_REG_A	; Turn enable on, set control signals
	STY VIA_REG_A	; Turn enable off

	jmp print_a		; Print the letter A forever

my_string:
	DC.B "Hello, world!", 0


	ORG $FFFA
reset_vectors:
	DC.W start	; NMI
	DC.W start	; RESET
	DC.W start	; IRQ
