ORG		0x7c00
bits	16



	
times 446-($-$$) db 0x00
;partition table begin
entry1:
	bootflag: 		db	0x80
	starthead:		db	0x00
	startsector:	db 	0x01
	startcylind:	db	0x00
	parttable:		db	0x0e
	endhead:		db 	0xff
	endsect:		db  0xff
	endcylind:		db	0xff
	startlba:		dd  0x00000001
	sizeinsect:		dd 	0x00000300
entry2:
	bootflag2: 		db	0x00
	starthead2:		db	0x00
	startsector2:	db 	0x01
	startcylind2:	db	0x00
	parttable2:		db	0x83
	endhead2:		db 	0xff
	endsect2:		db  0xff
	endcyli2nd:		db	0xff
	startlba2:		dd  0x00000301
	sizeinse2ct:		dd 	0x00000400
entry3:
	bootflag3: 		db	0x00
	starthea3d:		db	0x00
	startsect3or:	db 	0x01
	startcyli3nd:	db	0x00
	parttable3:		db	0xab
	endhead3:		db 	0xff
	endsect3:		db  0xff
	endcyli3nd:		db	0xff
	startlb3a:		dd  0x00000500
	sizeinse3ct:		dd 	0x00000550
entry4:
	times 16 db 0x00
;partition table end
dw 0xaa55
