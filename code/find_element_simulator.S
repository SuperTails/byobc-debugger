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
	; Write a program that finds a value in an array.
	;
	; For example, if the array was [42, 17, 22, 53, -7, 0], and the A register contains 53,
	; the program should finish with 3 in the X register, since 53 is the third element.
	;
	; You can fill in search and search_loop to do this.
	; The array is stored at array_data.
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
	; If the number below XR is "03", you're done!
	;
	; I'm only testing this on one array, but you should write the code to work on any array.
	; You are allowed to assume elements never occur twice.
	; You are also allowed to assume the array always has at least 1 element.
	;
	; For 1 point of extra credit, make your program handle empty arrays
	; and arrays that don't contain the element by returning -1 ($FF).
	; You can assume all the arrays end with a 0.

	.org $8000

start:
	lda #53			; This is the number you are searching for. Don't change this line!

	; you can put code here...

search_loop:

	; and you can also put code here ...

halt:
	jmp halt 	; This is the end of the program; the processor will stay in this loop forever

; This is the array you're searching through.
array_data:
	.BYTE	42, 17, 22, 53, -7, 0