# include "../io/io.h"
# include "disk.h"
int read_sect_disk(int lba, int total, void* buf)
{
		//lba contains starting lba number to read form
		//total represents total number of blocks to read
		//buf points to address where this block inputed that has to be stored
		outb(0x1f6, 0xe0 | ((lba >> 24) & 0x0f)); //we fill the lower 4 bits with 24-27bits of lba and upper for bits with 0xe which denotes master
		outb(0x1f2,total);
		outb(0x1f3,(lba &0xff));	//lower one byte of lba to 0x1f0
		outb(0x1f4,((lba >> 8) & 0xff));	//bytes 8 to bytes 15 of lba
		outb(0x1f5,((lba >>16 ) & 0xff)); //bytes 16 to 23 of lba
		outb(0x1f7,0x20);
		uint16_t * ptr=(uint16_t*)buf;
		for(int i=0;i<total;i++)
		{
			char flg= inb(0x1f7);
			while((flg & 0x88)!=0x08)  //flg&0x88 ==0x08
				flg=inb(0x1f7);
				
			for(int k=0;k<256;k++,ptr++)
				*ptr= in16(0x1f0);
		}
		return 0;
}
