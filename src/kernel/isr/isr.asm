section .asm

extern divide_zero
extern int21_handler
extern no_interrupt

global sr0
global sr21
global isr_x
global sr20
sr0:	
	pushad
	call	divide_zero
	jmp		$
	popad
	iretd

sr21:
	cli
	pushad
	
	call 	int21_handler
	
	;send EOI to port 
	mov		al,0x20
	out 	0x20,al
	popad
	
	sti
	iretd
	
sr20:
	pushad
	
	mov	al,0x20
	out	0x20,al
	popda
	iretd
isr_x:
	cli
	pushad
	
	call 	no_interrupt
	
	;send EOI to port 
	mov		al,0x20
	out 	0x20,al
	popad
	
	sti
	iretd
