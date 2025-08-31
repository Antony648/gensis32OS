org 0x7c00
bits 16

start:
    xor  ax,ax
    mov  es,ax
    mov  ss,ax
    mov  gs,ax
    mov  ds,ax
    mov  ax,0x7c00
    mov  sp,ax
    
print_subroutine:
    mov  ah,0x0a
    mov  al,'W'
    mov  bh,0x00
    mov  cx,0x05
    int  0x10

jmp  $

times 510-($-$$) db 0x00
dw  0xaa55
