section .asm
global load_idtr

load_idtr:
	push	ebp		
	mov 	ebp,esp
				;standard function prologue creates a new stack frame 
	mov	ebx,[ebp+8]	;loads value of first parameter to ebx ebp=esp , last pushed value is rtn addr, and value before that is first parmeter
	lidt	[ebx]
				;standard function epilogue
	pop	ebp
	ret
