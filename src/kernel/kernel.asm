[BITS 32]
global _start
global problem
extern kernel_main
_start:
	mov	ax,0x10
	mov	ss,ax
	mov	es,ax
	mov	gs,ax
	mov	fs,ax
	mov 	ds,ax
	mov 	ebp,0x200000
	mov 	esp,ebp

	in	al,0x92
	or	al,0x02
	out	0x92,al
	call	kernel_main
	jmp $
	
problem:
	;push	ebp
	;mov		ebp,esp
	
	int  	0x20
	;pop		ebp
	;ret
times 512-($-$$) db 0x0

;this is to ensure that the code occupies entire sector and not just some parts for eg if you add it behind the bootloader you will see that it just takes up some bytes it does not fillup the sector, this is not good because it will cause lot of alignment issues,
; refer notes on alignment in /notes/alignment_text for detailed info
