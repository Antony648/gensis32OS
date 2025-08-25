# include "../io/io.h"
# include "disk.h"
# include "../essentials/essentials.h"
# include "../osconfig.h"
# include "../error.h"
# include "../heap/heap_cream.h"
# include "../time/time.h"
# include "../kernel.h"
uintptr_t karray[5]={0,0,0,0,0};

#include "stdbool.h"
extern uint8_t G_BOOT_DRIVE;
struct disk disk1;
struct disk* motherlobe[5]={NULL,NULL,NULL,NULL,NULL};
/*int read_sect_disk(uint32_t lba, uint32_t total, void* buf)
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
}*/
void set_disk_info(struct disk* diske)
{
	uint16_t ata_val=diske->ata_code;
	unsigned short head,command_port,base,control_port,drive;
	if(!(ata_val& 0x02))	//check for primary
		base=0x1f0;
	else	
		base=0x170;
	
		
	control_port=base + 0x206;
	command_port=base+0x07;
	head=base+0x6;
	//status port command are one and same acts like status when we try to read , acts like command when we write
	//data and base port are same
	if(!(ata_val&0x01))	//check master
		drive=0xa0;
	else
		drive=0xb0;
	
	diske->drive_head=drive;
	if (!(drive &0x10)) diske->is_master=1;
	else diske->is_master=0;
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
	
	for(int i=0;i<4;i++)
			inb(base+6);
	
	
	int k=ATA_WAIT;
	while(inb(command_port)&0x80)
	{
		if(--k==0)
			return ;
	}
	outb(command_port,(unsigned char)command);	//this will set the command
	
	k=ATA_WAIT;
	while((inb(command_port) & 0x80) || !(inb(command_port)&0x08))
	{
		if(--k==0)
			return;
	}
	for(int i=0;i<256;i++)
		buf[i]=in16(base);
	
}
struct disk* check_disk(unsigned short ata_val)
{
	struct disk* rtn_val=NULL;
	uint16_t port_no,drive;
	print("trying");
	print_16(ata_val);
	print("\n");
	
	port_no=(ata_val & 0x02)?0x170:0x1f0;
	drive=(ata_val & 0x01)?0xb0:0xa0;
	
	outb(port_no+0x206,0x00);	//clearing control 
	outb(port_no+6,drive);	//setting communication channel
	
	//delay
	for(int i=0;i<4;i++)
		inb(port_no+6);
	
	print("communication set for"); print_16(ata_val);print("\n");
	
	uint8_t stat=inb(port_no+7);
	if(stat ==0xff || stat==0x00)
	{
		print("no device \n"); return NULL;
	}
	
	int k=ATA_WAIT;
	while(inb(port_no+7) &0x80)
	{
		if(k==0)
			return NULL;
		k--;
	}
	//
	outb(port_no+7,0xec);
	print("info command send for"); print_16(ata_val);print("\n");
	//wait
	for(int i=0;i<4;i++)
		inb(port_no+6);
		
	k=ATA_WAIT;	
	while((inb(port_no+7) & 0x80) || !(inb(port_no+7)& 0x08))
	{
		if(k==0)
			return NULL;
		k--;
	}
	
	uint8_t status =inb(port_no +7);
	print("status obtained for"); print_16(ata_val);print("\n");
	print("status:"); print_hex(status);print("\n");
	k=ATA_WAIT;
	while(!(status & 0x08))
	{
		status =inb(port_no+7);
		if(k==0)
			return NULL;
		k--;
	}
	if(status==0x00 || status== 0x01)
		return rtn_val;
	if(status & 0x08)
	{
		print("detected one more disk...\n");
		rtn_val=(struct disk*)heap_cream_malloc(karray,sizeof(struct disk));
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
	
	//get size from our disk
	uint16_t* buf=heap_cream_malloc(karray,512);
	set_disk_info(&disk1);
	get_disk_info(&disk1,0xec,buf);
	//should contain data in buffer at this point
	disk1.sect_count=(uint32_t)(buf[61]<<16)|(uint32_t)buf[60];
	//disk1.sect_size=(uint32_t)(buf[118]<<16)|(uint32_t)buf[117];
	disk1.sect_size=512;
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
			//val->sect_size=(uint32_t)(buf[118]<<16)|(uint32_t)buf[117];
			val->sect_size=512;
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
	//check for trying to read outside bounds
	if(lba+total > disk_p->sect_count)
		return -GEN32_INVARG;
	unsigned short base=disk_p->base_data_port;
	if(disk_p->is_master)
		outb(base+6,0xe0|((lba>>24)&0x0f));
	else
		outb(base+6,0xf0|((lba>>24)&0xf));
		
	int k=ATA_WAIT;
	while(inb(base+7) & 0x80)
	{
		//sleep(10) // will uncommet after setting up int32 to throw someting an puttin proper isr
		//to wait for 10 sec,for now a curde sleep
		if(k==0)
			return -TIMEOUT;
		k--;
	}
	//check 
	outb(base+2,total&0xff);
	
	outb(base+3,(lba &0xff));
	outb(base+4,((lba>>8)&0xff));
	outb(base+5,((lba>>16)&0xff));
	
	k=ATA_WAIT;
	while(inb(base+7) & 0x80)
	{
		//sleep(10) // will uncommet after setting up int32 to throw someting an puttin proper isr
		//to wait for 10 sec,for now a curde sleep
		if(k==0)
			return -TIMEOUT;
		k--;
	}
	outb(base+7,0x20);	//code 0x20 means to read
	
	for(int i=0;i<450;i++)
		asm volatile ("nop");
	
	k=ATA_WAIT;
	while(inb(base+7) & 0x80)
	{
		//sleep(10) // will uncommet after setting up int32 to throw someting an puttin proper isr
		//to wait for 10 sec,for now a curde sleep
		if(k==0)
			return -TIMEOUT;
		k--;
	}
	
	
	//here we are ready to accept data from data port
	uint16_t *cur_start=(uint16_t*)buf;
	for(int i=0;i<total;i++)
	{
		k=ATA_WAIT;
		while((inb(base+7)&0x88)!=0x08)
		{
			if(k==0)
				return -TIMEOUT;
			k--;
		}
	
		//check for errros
		if(inb(base+7)&0x01 || inb(base+7)&0x20) 
			return DISK_READ_ERR;
		for(int j=0;j<256;j++)
		{
			cur_start[j]=in16(base);
		}
		cur_start+=256;
		
	}
	return 0;
}
void disk_debug_print()
{
	for(int i=0 ;i<4;i++)
		if(motherlobe[i]!=NULL)
		{
			print("ata_code:");print_32(motherlobe[i]->ata_code);print("\n");
			print("base/data port:"); print_16(motherlobe[i]->base_data_port);print("\n");
			print("control port:");print_16(motherlobe[i]->control_port);print("\n");
			print("command/status port:");print_16(motherlobe[i]->command_status_port);print("\n");
			print("head:");print_16(motherlobe[i]->head);print("\n");
			print("drive head:");print_hex(motherlobe[i]->drive_head);print("\n");
			print("is master:");print_hex(motherlobe[i]->is_master);print("\n");
			print("sector count:");print_32(motherlobe[i]->sect_count);print("\n");
			print("sect size:");print_32(motherlobe[i]->sect_size);print("\n");
		}
	
}
