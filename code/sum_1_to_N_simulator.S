	; As a refresher, here's the instructions we've covered so far:
	;
	; --- Load Instructions ---
	; Mnemonics: LDA, LDX, LDY
	; Example:
	; 		LDA #42     	Loads the value 42 into A
	;		LDX 7			Loads the value at address 7 into X
	;
	; --- Store Instructions ---
	; Mnemonics: STA, STX, STY
	; Example:
	;		STA 9			Stores the value in A to address 9
	;		STX $1B			Stores the value in X to address $1B
	;
	; --- Increment, Decrement Instructions ---
	; Mnemonics (increment): INC A, INX, INY
	; Mnemonics (decrement): DEC A, DEX, DEY
	; Example:
	;		INX				Increases the value in X by 1
	;		DEY				Decreases the value in Y by 1
	;
	; --- Flag Change Instructions ---
	; Mnemonics: CLC, SEC
	; Example:
	;		CLC				Clears the carry status flag
	;		SEC				Sets the carry status flag
	;
	; --- Addition, Subtraction Instructions ---
	; Mnemonics: ADC, SBC
	; Example:
	;		CLC
	;		ADC #42			Adds 42 to the value stored in A
	;
	;		SEC
	;		SBC $2A			Subtracts the value at address $2A from A
	;
	; --- Jump, Branch Instructions ---
	; Mnemonics: BNE JMP
	; Example:
	;		BNE foobar		Jumps directly to foobar if the Zero flag is 0
	;		JMP baz			Jumps directly to baz

	; === Steps ===
	;
	; Fill in sum_loop so that, after the loop finishes,
	; the A register will contain the sum of all numbers from 1 to 10.
	; You can do this using only 5 instructions from the above list!
	;
	; Once you have something you think will work,
	; paste this whole file into this assembler website: https://www.masswerk.at/6502/assembler.html.
	; Hit "Assemble", and then "Show In Emulator".
	;
	; In the emulator, click the number below the "PC" and type in 8000 if it's not already 8000.
	; Click the checkbox in the top right that says "trace".
	;
	; Then click the "single step" button repeatedly until you see
	; that you've reached the JMP at the end.
	;
	; If the number below AC is "37" (which is 55 in decimal), you're done!
	;

	.org $8000

start:
	ldx #10			; X will be your N, this counts down to 0.
	lda #0			; A should contain your running total and then final answer.

sum_loop:
	; Your code goes here!

	; Add X to A

	; Decrement X

	; Branch back up to the beginning of the loop!

	
halt:
	jmp halt 	; This is the end of the program; the processor will stay in this loop forever