org		0x7c00
bits	16

jmp		short	start 
nop
OEM_idenifier:		db	'MSWIN4.1'
bytes_per_sect:		dw	512
sect_per_clust:		db	1
no_res_sect:		dw	1
fat_no:		 	db 	2
root_dir_entries:	dw	0xe0
no_sectors:		dw	2880
media_desc:		db	0x0f0	
sect_per_fat:		dw 	9
sect_per_track:		dw 	18
heads:			dw 	2
hiddensect:		dd	0
large_sect_no:		dd 	0
;extended boot record
drive_no:		db	0
flag:			db 	0
signature:		db 	0x29
volume_id:		db	0x12,0x34,0x56,0x78
volume_label:		db	'antony   os'
identifier_str:		db 	'fat12   '
start:
	jmp 	main
nop
;expects name of file to be read in si
;INT 0x13, AH = 2 -- read floppy/hard disk in CHS mode
;AH = 0x02 → Function number (Read sectors)
;AL = Number of sectors to read (must be between 1-127)
;CH = Cylinder number (tracks, 0-based)
;CL = Sector number (1-63) → Bits:
;   Bits 0-5 → Sector number (1-63, not 0-based)
;   Bits 6-7 → High bits of Cylinder number (if needed)
;DH = Head number (0-1 for a floppy disk)
;DL = Drive number (0 for first floppy, 1 for second)
;ES:BX = Buffer address → Where data should be stored

;expects sector number in cl and buffer location to be in bx
read_sector:
	pusha
	mov		ah,0x02
	mov		al,1			;no ofsector tobe read	
	mov 		dl,[drive_no]		
	mov 		dh,0x0			;head
	mov		ch,0x0 			;cylinder

	INT		0x13
	jc		.error_handling
	popa
.error_handling:
	ret

;setup_:
;set all register in condition expected by the system
;jmp control to region in ram that stores the buffer of kernel.bin
;so that cpu exectues it
;	ret
;kern:	db		"KERNEL   BIN"
start_boot:
;reseverd sector number =1
;fat region=no_fat*sect_per_fat
;root_reg_sect=reserv_sect+fat_region
;we copy directory entries into buffer(or direxcly access from img)
;and compare the name with "KERNEL  BIN" bad practice but we will fix later
;then copy sectors from kenel.bin file to buffer in ram
;call setup_
	mov	cl,[no_res_sect]
	mov	al,[fat_no]
	mov 	bl,[sect_per_fat]
	mul	bl
	add	cl,al
	;calculate the total number of sectors in root dir
	mov	ax,[root_dir_entries]
	mov	bx,28
	mul	bx
	mov	bx,512
	div	bx	;at this point number ofsectors in root dir is at ax
.loop:
	mov	bx,0x0500
	call	read_sector
	;compare from here
	inc	cl
	cmp	cl,al
	jle	.loop
	ret



;expects string data in si
;INT 0x10, AH = 0xE,AL=char val,bh=0x0 for printing in BIOS
print:
	push  	ax	;ax=ah,al
	push	bx
	push 	si
	mov		al,[si]
.loop:
	test 		al,al
	jz		.end
	mov		ah,0x0e
	mov 		bh,0x00
	INT		0x10
	lodsb
	jmp		.loop
.end:
	pop		si
	pop 		bx
	pop		ax
	ret
msg:	db 'success',0
msg1:	db  'fail',0
;error_handling:
;	mov	si,msg1
;	call	print
;	ret
main:
	;set all segment registers to 0
	;we cannot direcly push 
	xor 	ax,ax
	mov		ss,ax
	mov		ds,ax
	mov 	es,ax
	mov 	sp,0x7c00

	mov 	si,msg
	call	print
	hlt
.halt:
	jmp 	.halt

times 510-($-$$) db 0
dw 	0xaa55
