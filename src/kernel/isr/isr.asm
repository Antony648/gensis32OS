section .asm

extern divide_zero
global sr0
sr0:	
	pusha
	call	divide_zero
	jmp		$
	popa
	iret
