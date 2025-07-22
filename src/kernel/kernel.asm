[BITS 32]
global _start
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

	jmp $
