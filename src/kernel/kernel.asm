[BITS 32]
global _start

extern kernel_main

global enable_interrupts
_start:
	mov	ax,0x10
	mov	ss,ax
	mov	es,ax
	mov	gs,ax
	mov	fs,ax
	mov 	ds,ax
	
	in	al,0x92
	or	al,0x02
	out	0x92,al
	
iwc1:
	;set master and slave to init mode so we can remap them set up master-slave connection and put them to a 8086 mode
	mov		al,0x11
	out		0x20,al		;setup master pci 
	out 	0xa0,al
	
iwc2:
	;now we remap 
	mov		al,0x20
	out		0x21,al		;mov 0x20 to port 0x21
	mov		al,0x28
	out		0xa1,al		;mov 0x28 to port 0xa1 
	
iwc3:
	; we pass a value to the master containing only a single bit one, the bit that is high corresponds 
	; to  say that  slave is at irq2 we need to set master to value 100b or 0x04
	mov		al,0x04
	out 	0x21,al
	
	;to set slave to irq2 we need to pass the value as such 
	mov		al,0x02
	out 	0xa1,al
	
iwc4:
	;set master and slave to 8086 mode
	mov		al,0x01
	out 	0x21,al
	out 	0xa1,al
	
unmask_keyboard:
	in		al,0x21
	and 	al,0xfd
	out	 	0x21,al
	
	movzx	eax,dl
	push	eax
	call	kernel_main
	add		esp,4
	jmp $

enable_interrupts:
	push	ebp
	mov		ebp,esp
	sti
	pop		ebp
	ret	

times 512-($-$$) db 0x0

;this is to ensure that the code occupies entire sector and not just some parts for eg if you add it behind the bootloader you will see that it just takes up some bytes it does not fillup the sector, this is not good because it will cause lot of alignment issues,
; refer notes on alignment in /notes/alignment_text for detailed info
