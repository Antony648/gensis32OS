[bits 32]
section .asm

global enable_paging
global load_dir_table


load_dir_table:
	push 	ebp
	mov		ebp,esp
	mov		eax,[ebp+8]
	mov		cr3,eax
	pop		ebp
	ret

enable_paging:
	push 	ebp
	mov		ebp,esp
	mov		eax,cr0
	or		eax,0x80000000
	mov		cr0,eax 
	pop		ebp
	ret
