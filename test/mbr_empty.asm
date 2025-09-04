ORG		0x7c00
bits	16



	
times 446-($-$$) db 0x00
;partition table begin
entry1:
	times 16 db 0x00
entry2:
	times 16 db 0x00
entry3:
	times 16 db 0x00
entry4:
	times 16 db 0x00
;partition table end
dw 0xaa55
