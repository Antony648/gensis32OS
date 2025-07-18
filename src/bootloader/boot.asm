ORG	0X7c00
[bits	16]

jmp	start
start:
	cli
	mov	ax,0x00
	mov	ds,ax
	mov	es,ax
	mov	ss,ax
	mov   	sp,0x7c00
	sti
;GDT
load_gdt:
	cli
	lgdt 	[gdt_descriptor]
	;setting protection bit to 1
	mov 	eax,cr0
	or	eax,0x1
	mov	cr0,eax
	jmp	0x8:protected_mode
gdt_null:
	dd 	0x00 	;dword of 0s,32bits of 0s
	dd  	0x00 	;dword of 0x,32bits of 0s

gdt_kernel_code:	 ;meant for cs selector
	;base address is 0x0000 0000
	;segment limit is 0xffff f
	dw	0xffff	;lower 16bits of segment limit
	dw	0x0000	;lower 16bits of base address
	db	0x00	;middle 8bits of base  address
	db	0x9a	;b1000 1010 kernel level system,only excutable,only readable
	db	0xcf 	;b1100 1111 granularity=1(4KiB),protectmod=1, long=0,rserverd=0,
	db 	0x00

gdt_kernel_data:	;meant of ds,ss,es,gs
	 ;base address is 0x0000 0000
	 ;segment limit is 0xffff f
	dw 	0xffff
	dw 	0x0000
	db	0x00
	db	0x92	;b1000 0010 
	db 	0xcf
	db 	0x00
gdt_descriptor:
	dw  gdt_descriptor- gdt_null -1
	dd  gdt_null
[BITS 32]
protected_mode:
	mov 	ax,0x10
	mov	es,ax
	mov	ds,ax
	mov 	gs,ax
	mov 	ss,ax
	mov 	fs,ax
	mov	ebp, 0x00200000
	mov 	esp,ebp
	jmp 	$

times 510-($-$$) db 0
dw	0xaa55
