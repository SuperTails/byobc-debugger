	PROCESSOR 6502

VIA_REG_B		= $6000
VIA_REG_A		= $6001
VIA_DDR_B		= $6002
VIA_DDR_A		= $6003
VIA_T1_COUNT_L	= $6004
VIA_T1_COUNT_H	= $6005
VIA_T1_LATCH_L	= $6006
VIA_T1_LATCH_H	= $6007
VIA_SR			= $600A
VIA_AUX_CTRL	= $600B
VIA_PER_CTRL	= $600C
VIA_INT_FLAG	= $600D
VIA_INT_ENBL	= $600E

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


TIMER_PERIOD = [5000 / 2] - 1


; ========================================

	SEG		text
	ORG $8000

start:
	LDX #$FF		; Initialize the stack pointer
	TXS

	LDA #$FF		; Set all VIA pins to be outputs
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

	LDA #80					; Wait for the display to be ready again
delay_loop:
	ADC #1
	BCC delay_loop

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

	LDA #%00001001			; IRQB enabled, data terminal ready
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

init_timer:
	LDA #%01000000
	STA VIA_AUX_CTRL	; Set timer 1 free-running mode

	LDA #[TIMER_PERIOD & $FF]
	STA VIA_T1_COUNT_L
	LDA #[TIMER_PERIOD >> 8]
	STA VIA_T1_COUNT_H

	LDA #%11000000
	STA VIA_INT_ENBL	; Enable Timer 1 interrupt generation

	CLI					; Enable handling IRQs
halt:
	; Our code is free to do whatever we want in here,
	; without worrying about checking for new data or timers.
	; The interrupts will just handle it whenever necessary!
	JMP halt


; ================== ;
; Interrupt handlers ;
; ================== ;


irq_handler:
	PHA

	LDA ACIA_STATUS
	AND #[1 << 3]
	BNE acia_irq

	LDA VIA_INT_FLAG
	AND #[1 << 6]
	BNE timer_irq

	PLA
	RTI

timer_irq:
	LDA VIA_T1_COUNT_L	; Clear pending interrupt

	LDA VIA_REG_A		; Toggle A0
	EOR #1
	STA VIA_REG_A

	PLA
	RTI

acia_irq:
	LDA ACIA_DATA
	STA VIA_REG_B

	LDA VIA_REG_A
	ORA #[LCD_EN_HIGH | LCD_RS_DATA]
	STA VIA_REG_A
	AND #[~LCD_EN_HIGH]
	STA VIA_REG_A

	PLA
	RTI


; ======================== ;
; Constants and other data ;
; ======================== ;

my_string:
	DC.B "Hello, world!", 0

	ORG $FFFA
vector_table:
	DC.W start			; NMI
	DC.W start			; RESET
	DC.W irq_handler	; IRQ
