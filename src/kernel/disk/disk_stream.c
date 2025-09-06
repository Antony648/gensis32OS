#include "disk_stream.h"
#include "../heap/kheap.h"
#include "../osconfig.h"
#include "../kernel.h"
#include <stdint.h>

/*
int read_single_sect_disk(uint32_t lba, void* buf)
{
	//this function reads a single sector for to the sector number passed in
	//lba number
		//lba contains starting lba number to read form
		//buf points to address where this block inputed that has to be stored
		outb(0x1f6, 0xe0 | ((lba >> 24) & 0x0f)); //we fill the lower 4 bits with 24-27bits of lba and upper for bits with 0xe which denotes master
		outb(0x1f2,1);
		outb(0x1f3,(lba &0xff));	//lower one byte of lba to 0x1f0
		outb(0x1f4,((lba >> 8) & 0xff));	//bytes 8 to bytes 15 of lba
		outb(0x1f5,((lba >>16 ) & 0xff)); //bytes 16 to 23 of lba
		outb(0x1f7,0x20);
		uint16_t * ptr=(uint16_t*)buf;
		
		char flg= inb(0x1f7);
		while((flg & 0x88)!=0x08)  //flg&0x88 ==0x08
			flg=inb(0x1f7);
			
		for(int k=0;k<256;k++,ptr++)
			*ptr= in16(0x1f0);
	
		return 0;
}
*/
struct disk_stream* init_disk_stream(uint32_t disk_id)
{
	struct disk_stream *ds=kzalloc(sizeof(struct disk_stream));
	struct disk* disk1=get_disk(disk_id);
	if (!disk1 )
		return NULL;
	ds->disk_struct=disk1;
	ds->pos=0;
	return ds;
}
void disk_stream_seek(struct disk_stream* ds, uint32_t position)
{
		ds->pos=position;
}
void get_bytes_from_disk( struct disk_stream *ds, uint32_t no_bytes, uint8_t* ram_location)
{
	if(no_bytes ==0)
		return;
	
	uint8_t* buf=kzalloc(SECTOR_SIZE);
	uint32_t lba=ds->pos/SECTOR_SIZE;
	uint32_t index=ds->pos%SECTOR_SIZE;
	uint32_t i=0;
	uint32_t count = (no_bytes + index)/SECTOR_SIZE;
	if((no_bytes +index)% SECTOR_SIZE)
		count++;
	for(;count>0;count--,lba++)
	{
		if(read_disk_block(ds->disk_struct,lba,1,buf))
		{
			print("disk_stream.c:get_bytes_from_disk,failed to read the block");
			kfree(buf);
			return;
		}
		
		for(;i<no_bytes && index<SECTOR_SIZE;i++,index++)
		{
			ram_location[i]=buf[index];
		}
		index=0;
	}
	kfree(buf);
	return;
}
void free_disk_stream(struct disk_stream* ds)
{
	kfree(ds);
}

