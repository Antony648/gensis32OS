THE CURRENT IMPLEMENTATIONS HAVE LIMITATIONS 
FROM MY POINT OF VEIW THAT WILL NEED IMPORVEMENT


once the project is complete in its basic model I would like to implement these
(note: these are made from the current stage of development, there is a chance that  the tutor himself fixes these later on)
paging:
    the current implementation has a two level paging, but allocating 4mb+ 4kb at once, the reason that we implement two level paging is the non avilablilty of continuous 4mb , so we need to implement something like lazy paging

we need to fully leverage that

filesystem:
    we implement a FAT16, the cluster size if fixed to 128 sectors, this is not good as it will waste a lot of space for a small file,and number of max entries is 64, this is too low this limits number of files and subdirectories as 64 
    we need to fix that 
also planning to implement FAT32

also the kernel is written into the reserved sector this not how it should be actually implemented we need to write asm code for bootloader to read and load files from fat16 , so we can set store kernel.bin as a file instead of writing it in reserved sector acutally meant for bootloader
when we do that should reduce the cluster size as it may potentially overwrite data
