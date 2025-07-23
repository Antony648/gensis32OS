MODIFIED BOOTLOADER TO LOAD KERNEL CODE TO RAM AND JUMP TO IT
    
WORKDONE:
    modified bootloader to write an ata driver for reading 100 sectors excludinig mbr starting from address 0x100000(1MiB)
    then jumped to that location in ram to execute kernel code
    ensured that kernel code is executin successfully by using gdb
NOTES:
    [notes on ata driver](../notes/ata_driver)    
TOOLS:
    vim,qemu,gdb



