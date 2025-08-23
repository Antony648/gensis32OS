# include "../io/io.h"
# include "disk.h"
# include "../essentials/essentials.h"
# include "../osconfig.h"
# include "../error.h"
# include "../heap/heap_cream.c"
uintptr_t karray[5]={0,0,0,0,0};
#include "./disk.h"
#include "stdbool.h"
extern G_BOOT_DRIVE;
struct disk disk1;
struct disk* motherlobe[5]={NULL,NULL,NULL,NULL,NULL};
int read_sect_disk(uint32_t lba, uint32_t total, void* buf)
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
void set_disk_info(struct disk* diske)
{
	uint16_t ata_val=diske->ata_code;
	unsigned short status_port,head,command_port,base,control_port,drive;
	if(!(ata_val& 0x02))	//check for primary
	{ 
		diske->is_master=1;
		base=0x1f0;
	}
	else
	{
		diske->is_master=0;
		base=0x170;
	}
		
	control_port=base | 0b1000000110;
	command_port=base+0x07;
	head=base+0x6;
	//status port command are one and same acts like status when we try to read , acts like command when we write
	//data and base port are same
	if(!(ata_val&0x01))	//check master
	{
		diske->drive_head=0xa0;
		drive=0xa0;
	}
	else
	{
		diske->drive_head=0xb0;
		drive=0xb0;
	}
	//by here all value will be set
	diske->base_data_port=base;
	diske->command_status_port=command_port;
	diske->control_port=control_port;
	diske->head=head;
}
void get_disk_info(struct disk* diske,uint8_t command,uint16_t* buf)
{
	
	uint16_t base,command_port,control_port,head_port,drive;
	//
	base=diske->base_data_port;
	command_port=diske->command_status_port;
	control_port=diske->control_port;
	head_port=diske->head;
	drive=diske->drive_head;	
	//
	outb(control_port,0);
	outb(head_port,(unsigned char)drive);	//sets up channel for communication
	
	for(int i=0;i<400;i++)
			asm volatile("nop");// wait for 400ns
	
	
	
	while(inb(command_port)&0x80)
	{}
	outb(command_port,(unsigned char)command);	//this will set the command
	
	while((inb(command_port) & 0x80) || !(inb(command_port)&0x08))
	{}
	for(int i=0;i<256;i++)
		buf[i]=in16(base);
	
}
struct disk* check_disk(unsigned short ata_val)
{
	disk* rtn_val=NULL;
	unsigned char port_no,drive;
	port_no=(ata_val & 0x02)?0x170:0x1f0;
	drive=(ata_val & 0x01)?0xb0:0xa0;
	outb(port_no+0x206,0x00);
	outb(port_no+6,drive);
	
	//delay
	for(int i=0;i<450;i++)
		asm volatile ("nop");
	
	
	while(inb(port_no) &0x80){}
	outb(port_no+7,0xec);
	//wait
	while((inb(port_no+7) & 0x80) || !(inb(port_no+7)& 0x08)){}
	
	uint8_t status =inb(port_no +7);
	while(!(status & 0x08))
	{
		status =inb(port_no+7);
	}
	if(status==0x00 || status== 0x01)
		return rtn_val;
	if(status & 0x08)
	{
		rtn_val=(disk*)heap_cream_malloc(sizeof(struct disk));
		rtn_val->ata_code=ata_val;
		rtn_val->type=DISK_TYPE_REAL;
	}
	return rtn_val;
}



void disk_search_and_init()
{
	//we know the first disk exits
	
	disk1.type=DISK_TYPE_REAL;
	disk1.ata_code=G_BOOT_DRIVE;
	disk1.size=0;
	//get size from our disk
	uint16_t* buf=heap_cream_malloc(karray,512);
	set_disk_info(&disk1);
	get_disk_info(&disk1,0xec,buf);
	//should contain data in buffer at this point
	disk1.sect_count=(uint32_t)(buf[60]<<16)|buf[61];
	disk1.sect_size=(uint32_t)(buf[117]<<16)|buf[118];
	motherlobe[0]=&disk1;
	//now we can set first entry of motherlobe
	//scan of other three
	int index=1;
	for(unsigned short i=0x81; i <0x84;i++)
	{
		struct disk* val=check_disk(i);
		if(val)
		{
			set_disk_info(val);
			get_disk_info(val,0xec,buf);
			//should contain value in buffer we can reuse buf
			val->sect_count=(uint32_t)(buf[61]<<16)|(uint32_t)buf[60];
			val->sect_size=(uint32_t)(buf[118]<<16)|(uint32_t)buf[117];
			motherlobe[index]=val;
			index++;
		}
	}
	//motherlobe should contain possible disks as entries
}
	


struct disk* get_disk(uint32_t index)
{
	return motherlobe[index];
	
}

int read_disk_block(struct disk* disk_p,uint32_t lba, uint32_t total, void* buf)
{
	if(disk_p != &disk1)
		return -GEN32_EIO;
	return read_sect_disk(lba,total,buf);
}
