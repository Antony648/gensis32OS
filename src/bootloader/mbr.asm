ORG		0x7c00
[BITS 16]
lba_save: dd 0x00
dl_val: db 0x00
dap:	times 16 db 0x00
fallback_rel equ fallback-0x1c00
jmp_here_rel equ jmp_here-0x1c00
start:
	mov 	ax,0x00
	mov		ss,ax
	mov		ds,ax
	mov		es,ax
	mov		sp,0x6000	;we are going to put stack before 0x6000
	mov		[dl_val],dl
	;so 0x6000 to 0x7c00 doesnot contian any data
	jmp		chs_end
lba_setup:
;expects lba_save
	mov		al,0x10
	mov		[dap],al
	mov		al,0x00
	mov		[dap+0x01],al
	mov		ax,1
	mov		word [dap+0x02],ax
	mov		ax,0x7c00			;can be hard codes as i am planning to use lba access only for vbr
	mov		word [dap+0x04],ax
	mov		ax,0x00
	mov		word [dap+0x06],ax
	
	lea		si,[lba_save]
	lea		di,[dap+0x08]
	mov		cx,0x04
	cld
	rep 	movsb
	;dap+0xc to end is aldready 0x00	
	mov		word [dap+0xc],0
	mov		word [dap+0xe],0		
	ret
.error_lba:
	jmp		$

chs:
;lba at lba_save
	
	mov		ah, 0x08
	mov		dl, 0x80
	int		0x13
	jc		.error
	
	xor		ax,ax
	mov		al,dh
	inc 	ax
	and		cl,0x3f	;sector in cl
	mul		cl
	
	;
	mov		bx,[lba_save]
	xchg	ax,bx
	
	xor 	dx,dx
	div		bx
	mov		bx,ax	;bx contains cylinder
	mov		ax,dx	;ax contains temp
	div		cl		
	;head in al
	inc		ah
	;sector in ah
	
	;set as param for int 0x13 ah=2 
	mov		cl,ah
	and		cl,0x3f
	mov		dh,al
	mov		ch,bl
	shl		bh,6
	or		cl,bh
	
		
	
	ret
.error:
	jmp		$
chs_end:
;code for copying mbr to 0x6000

;we use chs for mbr copy  because it is not 8gb away..
	
	mov		dword [lba_save],0		;mov lba number to bx
	call	chs
	;we expect the values as per requriement
	xor		ax,ax
	mov		es,ax
	mov		bx,0x6000	;location to load mbr into
	mov		al,1	;number of sectors to read
	mov		ah,2	
	mov		dl,[dl_val]
	int		0x13
;end of copying mbr to 0x600
jmp		jmp_here_rel	;0x1c00-4 because we want to jump to the very next instruction 
jmp_here:
					
;code for copying vbr to 0x7c00
;we try lba if fail we fallback to chs
	mov		dword [lba_save],1	;for now we hardcode the value as we know
lba_call:
	call 	lba_setup
	mov		si,dap
	mov		ah,0x42
	jmp  	call_act
fallback:
	mov		dword [lba_save],1
	call   	chs		;still works because the code at 0x7c00 is not destroyed
	mov		ah,2
	xor		ax,ax	;es is aldready 0x00 but ...
	mov		es,ax
	mov		bx,0x7c00	;location to load mbr into
	mov		al,1	;number of sectors to read
call_act:
	
	mov		dl,[dl_val]
	int		0x13
	
	jc		not_quite
	jmp		0x000:0x7c00
;end of code for copying to 0x7c00

	;mov  dl,[dl_val]	;vbr expects it
	;far jump to vbr code
not_quite:
	jmp		0x00:fallback_rel

	
times 446-($-$$) db 0x00
;partition table begin
entry1:
	bootflag: 		db	0x80
	starthead:		db	0x00
	startsector:	db 	0x01
	startcylind:	db	0x00
	parttable:		db	0x06
	endhead:		db 	0xff
	endsect:		db  0xff
	endcylind:		db	0xff
	startlba:		dd  0x00000001
	sizeinsect:		dd 	0x00773594
entry2:
	times 16 db 0x00
entry3:
	times 16 db 0x00
entry4:
	times 16 db 0x00
;partition table end
dw 0xaa55
