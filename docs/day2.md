Day2 - GDT and Real Mode Setup

WORKDONE:
    added gdt table to the bootloader,
    loader gdt table and far jumped to 32bit protected mode
    setup the selectors as well as stack and base pointers
    enabled  A20 line by setting A20 gate to high

NOTES:
    [theortical concepts](../notes/protected_mode)
    [program expalantion on theory grounds](../notes/enter_protect_mode)
    [theroy on A20 and systemports](../notes/A20_gat)

TOOLS:
    nasm,qemu,vim,hexdump,gdb
