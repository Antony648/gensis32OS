ORG	0X7c00
[bits	16]

jmp	short start
nop

OEMIdentifier 		db 'GEN32 OS'
bytespersect		dw	0x200
sectperclust		db	0x80
reservedsect		dw	0xc8
fatcopies			db	0x02
rootdirents			dw 	0x40
sectnum				dw	0x00
mediatype			db	0xf8
sectperfat			dw	0x100
sectpertrack		dw	0x20
numheads			dw	0x40
hiddensect			dd	0x00
sectbig				dd  0x773594

;extended BPB
drivenum			db	0x80
WinNTbit			db	0x00
sign				db  0x29
volID				dd	0xd105
volIDString			db	'GEN32  BOOT'
sysIDString			db  'FAT16   '

;fot separate use
boot_disk			db 
root_disk			db 0x81		;bootloader assumes root at disk 0x81
root_part			db 0x00 	;bootloader assumes rootfs at first partition of disk ata 0x81
start:
	cli
	mov	ax,0x00
	mov	ds,ax
	mov	es,ax
	mov	ss,ax
	mov   	sp,0x7c00
	mov		[boot_disk],dl
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
;setup the descriptors
	mov	ax,0x10
	mov	ss,ax
	mov	es,ax
	mov	gs,ax
	mov	fs,ax
	mov 	ds,ax
	mov 	ebp,0x200000
	mov 	esp,ebp
;done
	mov 	ebx,2		;lba number	
	mov 	ecx,100		;sector count
;setting es for es:di so that it can be evaluted well for insw and es selector points to second entry in GDT 
	mov	ax,0x10
	mov	es,ax

	mov 	edi,0x0100000
	call   	ata_lba_read
ata_lba_read:
;move sector count (as 8 bit value) to i/o port 0x1f2
;we are using dx to store port numbers because it is instruction encodeing to do so, that if you are using a port above 255 then mov to dx before out
	mov	dx,0x1f2	
	mov	al,cl
	out	dx,al	;cl should represent 100 at this point
	
;lower (0-8) bits or lba number to 0x1f3
	mov	dx,0x1f3
	mov	al,bl	;lower 8 bits of ebx 
	out 	dx,al 
;middle (8-15)bits of lba number to 0x1f4
	mov	dx,0x1f4
	mov	al,bh  
	out	dx,al

;upper (16-23)bits of lba number in 0x1f5
	mov	dx,0x1f5
	mov	eax,ebx
	shr	eax,16	;now eax contains 16bits of valid info al=ebx(16-23), and ah=ebx(24-31) we need al value and ebx(24-27) in al in next out
	out	dx,al

;(24-27)bits of lba number in 0x1f6
	mov	dx,0x1f6
	and	ah,0x0f	;now upper 4bits of ah are zero	
	or	ah,0xe0 ;now upper 4bits of ah contain 0xe
	mov	al,ah ;ah contains bits(24-31)ebx, so we move it to al
	out	dx,al


	mov	dx,0x1f7
	mov	al,0x20
	out 	dx,al
.call_read_sect:
	push	ecx	;100 is pushed to stack(init) it is the number of sectors we want to read below is the code fro reading a single sector
.ata_wait_busy_set_drq_clear:
	mov 	dx,0x1f7
.loop:
	in 	al,dx	;stores value of port 0x1f7 in al
	test	al,0x80	;compares value at al with '1000 0000b'so it checks if the 7th bit is set,if it is set then ATA is BSY(busy)
	jnz	.loop
	test 	al,0x08 ;if 3rd bit is not set then loop back
	jz	.loop

.read_sect:
	mov	cx,0x100	;512/2=256=0x100
	mov	dx, 0x1f0
	cld			;ensure read always goes forward
	rep	insw		;reads 16bit value form 0x1f0 and 0x1f1 and store them to memory address pointed to by es:di and es:di+1
;increment di by 2 and decrement cx by 1 and check if cx is 0 
;the cx part is done by rep, moving of 2 bytes from sector number in bin file to es:di in ram is done by insw
	pop 	ecx	
;	add	edi,0x200
	dec	ecx
	jnz	.call_read_sect

	mov		dl,[boot_disk]
jmp 	0x100000 	;jmps to location 1MiB in ram as cs is adready set
	
times 510-($-$$) db 0
dw	0xaa55
